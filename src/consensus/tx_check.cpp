// Copyright (c) 2017-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_check.h>

#include <consensus/amount.h>
#include <primitives/transaction.h>
#include <consensus/validation.h>

bool CheckTransaction(const CTransaction& tx, TxValidationState& state)
{
    // Basic checks that don't depend on any context
    if (tx.vin.empty())
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vin-empty");
    if (tx.vout.empty())
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-empty");
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(TX_NO_WITNESS(tx)) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-oversize");
    }

    // ===================================================================
    // CRITICAL CONSENSUS CHECK: Output Value Validation (CVE-2010-5139)
    // ===================================================================
    // This check prevents integer overflow attacks on transaction output values.
    //
    // Historical Context:
    // CVE-2010-5139 was an overflow bug where an attacker could create outputs
    // with values that, when summed, would overflow a 64-bit integer and wrap
    // around to a small or negative number, effectively creating unlimited coins.
    //
    // Example Attack (prevented):
    // Output 1: 92,000,000,000,000,000 satoshis
    // Output 2: 92,000,000,000,000,000 satoshis
    // Sum: 184,000,000,000,000,000 (would overflow MAX_MONEY check if done at end)
    //
    // Defense Pattern:
    // 1. Check each individual output ≤ MAX_MONEY (21M BTC)
    // 2. Accumulate total: nValueOut += txout.nValue
    // 3. Check total after EACH addition (not just at end)
    //
    // Why check after each addition?
    // - Prevents overflow from being exploited before final check
    // - Even if individual outputs are valid, sum could overflow
    // - int64_t max is ~9.2×10^18, MAX_MONEY is 2.1×10^15
    // - Multiple valid outputs could sum to overflow
    //
    // This pattern is REQUIRED for all monetary arithmetic in consensus code.
    // ===================================================================
    CAmount nValueOut = 0;
    for (const auto& txout : tx.vout)
    {
        if (txout.nValue < 0)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-txouttotal-toolarge");
    }

    // ===================================================================
    // CRITICAL CONSENSUS CHECK: Duplicate Input Detection (CVE-2018-17144)
    // ===================================================================
    // This check prevents a critical vulnerability where a transaction could
    // spend the same UTXO multiple times, leading to either:
    // 1. Node crash (in debug builds via assert in UpdateCoins)
    // 2. Inflation bug (creating coins from nothing)
    //
    // Historical Context:
    // CVE-2018-17144 was a critical vulnerability discovered in 2018 where
    // this check was accidentally removed. Without it:
    // - First spend of input: UpdateCoins() marks UTXO as spent
    // - Second spend of same input: UpdateCoins() tries to spend again
    // - Result: Crash or database corruption depending on implementation
    //
    // Defense Mechanism:
    // Using std::set for deterministic duplicate detection across all platforms.
    // Set insertion returns false if element already exists, providing O(log n)
    // duplicate detection that's consensus-compatible across all implementations.
    //
    // This check MUST run before UpdateCoins() is called to maintain UTXO
    // database integrity and prevent inflation attacks.
    // ===================================================================
    std::set<COutPoint> vInOutPoints;
    for (const auto& txin : tx.vin) {
        if (!vInOutPoints.insert(txin.prevout).second)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-inputs-duplicate");
    }

    if (tx.IsCoinBase())
    {
        if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100)
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-cb-length");
    }
    else
    {
        for (const auto& txin : tx.vin)
            if (txin.prevout.IsNull())
                return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-prevout-null");
    }

    return true;
}
