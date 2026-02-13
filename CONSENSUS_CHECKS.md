# Bitcoin Core Consensus Validation Flow

**Document Version:** 1.0  
**Last Updated:** 2026-02-13  
**Purpose:** Comprehensive documentation of consensus validation flow

---

## Table of Contents

1. [Overview](#overview)
2. [Validation Architecture](#validation-architecture)
3. [Block Validation Flow](#block-validation-flow)
4. [Transaction Validation Flow](#transaction-validation-flow)
5. [Script Validation Flow](#script-validation-flow)
6. [Input Validation Flow](#input-validation-flow)
7. [Consensus Rules Summary](#consensus-rules-summary)
8. [Validation State Machine](#validation-state-machine)
9. [Error Handling](#error-handling)
10. [Critical Invariants](#critical-invariants)

---

## Overview

Bitcoin consensus is enforced through a multi-layer validation system that checks blocks and transactions at different levels:

1. **Context-Independent Validation** - Checks that don't depend on chain state
2. **Context-Dependent Validation** - Checks that require chain state
3. **Script Validation** - Verification of spending conditions
4. **State Updates** - Applying valid changes to UTXO set

```
┌─────────────────────────────────────────────────────────────────┐
│                     Bitcoin Consensus Layers                     │
├─────────────────────────────────────────────────────────────────┤
│  Layer 1: Format Validation (Deserialization)                   │
│  Layer 2: Context-Free Checks (CheckBlock, CheckTransaction)    │
│  Layer 3: Context-Dependent Checks (ConnectBlock)               │
│  Layer 4: Script Execution (VerifyScript, CheckInputScripts)    │
│  Layer 5: State Updates (UpdateCoins, UpdateUTXOSet)            │
└─────────────────────────────────────────────────────────────────┘
```

---

## Validation Architecture

### Separation of Concerns

The validation code is organized by responsibility:

```
src/consensus/
├── tx_check.cpp      → Context-independent transaction checks
├── tx_verify.cpp     → Context-dependent transaction verification
├── merkle.cpp        → Merkle tree computation and validation
├── amount.h          → Money range validation
├── consensus.h       → Constants (MAX_BLOCK_WEIGHT, etc.)
└── params.h          → Network-specific parameters

src/
├── validation.cpp    → Block validation orchestration
└── script/
    └── interpreter.cpp → Script execution engine
```

### Design Principle

**Consensus-Critical vs Policy:**
- **Consensus:** Rules that all nodes MUST agree on (in `consensus/`)
- **Policy:** Local node policies that MAY differ (in `policy/`)

Example:
- Consensus: Block weight ≤ 4,000,000 bytes (ALL nodes enforce)
- Policy: Mempool size limit (nodes may configure differently)

---

## Block Validation Flow

### High-Level Block Processing

```
New Block Received
    ↓
[1] CheckBlockHeader()          ← Proof of work, basic header validation
    ↓
[2] AcceptBlockHeader()         ← Chain context, checkpoint validation
    ↓
[3] CheckBlock()                ← Context-free block checks
    ↓
[4] ContextualCheckBlock()      ← Witness commitment, weight, sigops
    ↓
[5] ConnectBlock()              ← Execute all transactions, update UTXO
    ↓
[6] UpdateTip()                 ← Activate new best chain
    ↓
Block Accepted
```

### Detailed Block Validation Steps

#### Step 1: CheckBlockHeader()

**File:** `src/validation.cpp:3874-3881`

```cpp
static bool CheckBlockHeader(const CBlockHeader& block, BlockValidationState& state, 
                              const Consensus::Params& consensusParams, bool fCheckPOW)
```

**Validates:**
- ✅ Proof of work (if `fCheckPOW == true`)
  - Checks `block.GetHash() < target` based on `block.nBits`
  - Uses `CheckProofOfWork()` from `pow.cpp`

**Does NOT validate:**
- ❌ Timestamp (checked later in contextual validation)
- ❌ Block version (checked in contextual validation)
- ❌ Merkle root (not needed for header-only validation)

**Returns:**
- `true` if header is valid
- `false` with `state.Invalid(BLOCK_INVALID_HEADER, "high-hash")` if PoW fails

---

#### Step 2: CheckBlock()

**File:** `src/validation.cpp:3964-4029`

```cpp
bool CheckBlock(const CBlock& block, BlockValidationState& state, 
                const Consensus::Params& consensusParams, bool fCheckPOW, bool fCheckMerkleRoot)
```

**Purpose:** Context-independent block validation.

**Validation Sequence:**

1. **Header Check** (line 3973)
   ```cpp
   if (!CheckBlockHeader(block, state, consensusParams, fCheckPOW))
       return false;
   ```

2. **Signet Block Solution** (lines 3977-3979) - Only for signet networks
   ```cpp
   if (consensusParams.signet_blocks && fCheckPOW && 
       !CheckSignetBlockSolution(block, consensusParams))
   ```

3. **Merkle Root** (lines 3982-3984)
   ```cpp
   if (fCheckMerkleRoot && !CheckMerkleRoot(block, state))
       return false;
   ```
   - Validates `block.hashMerkleRoot` matches computed root
   - Detects merkle tree malleability (CVE-2012-2459)
   - See [Merkle Tree Validation](#merkle-tree-validation) section

4. **Size Limits** (line 3993)
   ```cpp
   if (block.vtx.empty() || 
       block.vtx.size() * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT || 
       ::GetSerializeSize(TX_NO_WITNESS(block)) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
       return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-blk-length");
   ```
   
   **Checks:**
   - Block not empty
   - Transaction count × 4 ≤ 4,000,000
   - Serialized size (no witness) × 4 ≤ 4,000,000

5. **Coinbase Validation** (lines 3997-4001)
   ```cpp
   // First transaction must be coinbase
   if (block.vtx.empty() || !block.vtx[0]->IsCoinBase())
       return state.Invalid(..., "bad-cb-missing");
   
   // Rest must not be coinbase
   for (unsigned int i = 1; i < block.vtx.size(); i++)
       if (block.vtx[i]->IsCoinBase())
           return state.Invalid(..., "bad-cb-multiple");
   ```

6. **Transaction Validation** (lines 4005-4014)
   ```cpp
   for (const auto& tx : block.vtx) {
       TxValidationState tx_state;
       if (!CheckTransaction(*tx, tx_state)) {
           assert(tx_state.GetResult() == TxValidationResult::TX_CONSENSUS);
           return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, 
                               tx_state.GetRejectReason(), ...);
       }
   }
   ```
   
   **Critical:** Each transaction goes through `CheckTransaction()` which includes:
   - Duplicate input check (CVE-2018-17144)
   - Output value validation (CVE-2010-5139)
   - See [Transaction Validation Flow](#transaction-validation-flow)

7. **Signature Operation Count** (lines 4018-4023)
   ```cpp
   unsigned int nSigOps = 0;
   for (const auto& tx : block.vtx) {
       nSigOps += GetLegacySigOpCount(*tx);
   }
   if (nSigOps * WITNESS_SCALE_FACTOR > MAX_BLOCK_SIGOPS_COST)
       return state.Invalid(..., "bad-blk-sigops");
   ```
   
   **Note:** This is an underestimate. Full sigop counting happens in `ConnectBlock()`.

8. **Cache Result** (lines 4025-4026)
   ```cpp
   if (fCheckPOW && fCheckMerkleRoot)
       block.fChecked = true;  // Prevent re-validation
   ```

**Key Design Decisions:**

- ✅ **Early rejection:** Simple checks first, expensive checks later
- ✅ **Caching:** `block.fChecked` and `block.m_checked_merkle_root` prevent redundant work
- ✅ **No witness data:** Doesn't check witness at this stage (checked in `ContextualCheckBlock`)

---

#### Step 3: Merkle Tree Validation

**File:** `src/consensus/merkle.cpp`

**CheckMerkleRoot()** - Called from `CheckBlock()`

```cpp
static bool CheckMerkleRoot(const CBlock& block, BlockValidationState& state)
{
    if (block.m_checked_merkle_root) return true;  // Already validated
    
    bool mutated;
    uint256 merkle_root = BlockMerkleRoot(block, &mutated);
    
    // Check root matches
    if (block.hashMerkleRoot != merkle_root) {
        return state.Invalid(BlockValidationResult::BLOCK_MUTATED, 
                            "bad-txnmrklroot", "hashMerkleRoot mismatch");
    }
    
    // Check for malleability (CVE-2012-2459)
    if (mutated) {
        return state.Invalid(BlockValidationResult::BLOCK_MUTATED, 
                            "bad-txns-duplicate", "duplicate transaction");
    }
    
    block.m_checked_merkle_root = true;
    return true;
}
```

**BlockMerkleRoot() Algorithm:**

```cpp
uint256 BlockMerkleRoot(const CBlock& block, bool* mutated)
{
    std::vector<uint256> leaves;
    leaves.reserve((block.vtx.size() + 1) & ~1ULL);  // Round up to even
    
    // Collect transaction hashes
    for (size_t s = 0; s < block.vtx.size(); s++) {
        leaves.push_back(block.vtx[s]->GetHash().ToUint256());
    }
    
    return ComputeMerkleRoot(std::move(leaves), mutated);
}
```

**ComputeMerkleRoot() with Mutation Detection:**

```cpp
uint256 ComputeMerkleRoot(std::vector<uint256> hashes, bool* mutated)
{
    bool mutation = false;
    
    while (hashes.size() > 1) {
        // Detect duplicate adjacent hashes (CVE-2012-2459)
        if (mutated) {
            for (size_t pos = 0; pos + 1 < hashes.size(); pos += 2) {
                if (hashes[pos] == hashes[pos + 1]) 
                    mutation = true;  // Same hash twice!
            }
        }
        
        // Handle odd number of hashes
        if (hashes.size() & 1) {
            hashes.push_back(hashes.back());  // Duplicate last
        }
        
        // Hash pairs together
        SHA256D64(hashes[0].begin(), hashes[0].begin(), hashes.size() / 2);
        hashes.resize(hashes.size() / 2);
    }
    
    if (mutated) *mutated = mutation;
    if (hashes.size() == 0) return uint256();
    return hashes[0];
}
```

**CVE-2012-2459 Protection:**

The vulnerability: If transaction list is `[1,2,3,4,5,6]` or `[1,2,3,4,5,6,5,6]`, both produce the same merkle root because the last level duplicates tx 5+6.

```
     Original:              Malleated:
         A                      A
       /   \                  /   \
      B     C                B     C
     / \   / \              / \   / \
    D   E F   F            D   E F   F
   /|  /| |  |            /|  /| |\  |\
  1 2 3 4 5  6           1 2 3 4 5 6 5 6
```

**Defense:** Detect when `hashes[pos] == hashes[pos+1]` (same hash appears twice). This only happens with duplicate transactions in the block.

---

## Transaction Validation Flow

### Two-Stage Transaction Validation

```
Transaction Received
    ↓
┌─────────────────────────────────────────┐
│ Stage 1: Context-Independent Validation │  ← CheckTransaction()
├─────────────────────────────────────────┤
│ • Structural validity                   │
│ • Empty vin/vout check                  │
│ • Size limits                           │
│ • Output value validation               │
│ • Duplicate input detection             │
│ • Coinbase format                       │
│ • Prevout null check                    │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Stage 2: Context-Dependent Validation   │  ← CheckTxInputs()
├─────────────────────────────────────────┤
│ • Input availability (UTXO exists)      │
│ • Coinbase maturity (100 blocks)       │
│ • Input value range check               │
│ • Fee calculation (in - out)            │
│ • Double-spend prevention               │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Stage 3: Script Validation              │  ← CheckInputScripts()
├─────────────────────────────────────────┤
│ • Execute scriptSig + scriptPubKey      │
│ • Verify signatures                     │
│ • Check witness programs                │
│ • Validate spending conditions          │
└─────────────────────────────────────────┘
    ↓
Transaction Valid
```

---

### Stage 1: CheckTransaction()

**File:** `src/consensus/tx_check.cpp:11-60`

```cpp
bool CheckTransaction(const CTransaction& tx, TxValidationState& state)
```

**Purpose:** Context-independent validation (doesn't need chain state).

**Validation Sequence:**

#### 1. Empty Input/Output Check (lines 14-17)

```cpp
if (tx.vin.empty())
    return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vin-empty");
if (tx.vout.empty())
    return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-empty");
```

**Rationale:** Every transaction must have at least one input and one output.

---

#### 2. Size Limit Check (lines 19-21)

```cpp
if (::GetSerializeSize(TX_NO_WITNESS(tx)) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT) {
    return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-oversize");
}
```

**Why `TX_NO_WITNESS`?**
- Witness data hasn't been checked for malleability yet
- Base transaction size × 4 must fit in block
- Witness data checked separately in contextual validation

**Size Limit:**
- `MAX_BLOCK_WEIGHT = 4,000,000`
- Single transaction cannot exceed entire block weight

---

#### 3. Output Value Validation (lines 23-34) 

**🔒 CVE-2010-5139 Protection**

```cpp
CAmount nValueOut = 0;
for (const auto& txout : tx.vout) {
    // Check individual output not negative
    if (txout.nValue < 0)
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-negative");
    
    // Check individual output not too large
    if (txout.nValue > MAX_MONEY)
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-vout-toolarge");
    
    // Accumulate total
    nValueOut += txout.nValue;
    
    // Check total after each addition (overflow protection!)
    if (!MoneyRange(nValueOut))
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-txouttotal-toolarge");
}
```

**Critical Pattern:** Check after each addition!

**Why this matters:**
- `CAmount` is `int64_t` (signed 64-bit integer)
- Maximum value: 9,223,372,036,854,775,807 satoshis
- `MAX_MONEY = 2,100,000,000,000,000` satoshis (21M BTC)
- If an attacker creates outputs that sum to > `MAX_MONEY`, overflow could wrap to negative
- Checking after each addition catches overflow before it happens

**MoneyRange() Definition:**

```cpp
inline bool MoneyRange(const CAmount& nValue) { 
    return (nValue >= 0 && nValue <= MAX_MONEY); 
}
```

**Example Attack Prevented:**

```
Output 1: 9,000,000,000,000,000 satoshis (valid individually)
Output 2: 9,000,000,000,000,000 satoshis (valid individually)
Sum:     18,000,000,000,000,000 satoshis

✅ Detected: Sum > MAX_MONEY (2,100,000,000,000,000)
```

---

#### 4. Duplicate Input Detection (lines 36-45)

**🔒 CVE-2018-17144 Protection**

```cpp
// Check for duplicate inputs (see CVE-2018-17144)
// While Consensus::CheckTxInputs does check if all inputs of a tx are available, 
// and UpdateCoins marks all inputs of a tx as spent, it does not check if the 
// tx has duplicate inputs.
// Failure to run this check will result in either a crash or an inflation bug, 
// depending on the implementation of the underlying coins database.

std::set<COutPoint> vInOutPoints;
for (const auto& txin : tx.vin) {
    if (!vInOutPoints.insert(txin.prevout).second)
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-inputs-duplicate");
}
```

**Why this is CRITICAL:**

1. **Without this check:** A transaction could spend the same UTXO twice
2. **First spend:** `UpdateCoins()` marks UTXO as spent and adds value to available funds
3. **Second spend:** `UpdateCoins()` tries to spend already-spent UTXO
4. **Debug build:** `assert(is_spent)` crashes the node (line 2006 of validation.cpp)
5. **Release build:** Continues silently, corrupting UTXO database or inflating supply

**Example Attack:**

```
Transaction:
  Input 0: prevout = (txid:abc..., n:0)  → Spend 10 BTC
  Input 1: prevout = (txid:abc..., n:0)  → Spend SAME 10 BTC again!
  Output 0: 20 BTC                       → Create 20 BTC from 10 BTC

✅ Rejected: Duplicate input detected
```

**Data Structure:** `std::set<COutPoint>`
- Each `COutPoint` is `(txid, n)` tuple identifying a UTXO
- `insert().second` returns `false` if element already in set
- Deterministic behavior across all platforms

---

#### 5. Coinbase Format Check (lines 47-51)

```cpp
if (tx.IsCoinBase()) {
    if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100)
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-cb-length");
}
```

**Coinbase scriptSig Requirements:**
- Minimum 2 bytes (historically for block height after BIP34)
- Maximum 100 bytes (prevents bloat)
- Can contain arbitrary data (used for extra nonce, mining pool info, etc.)

---

#### 6. Non-Coinbase Prevout Check (lines 52-57)

```cpp
else {
    for (const auto& txin : tx.vin)
        if (txin.prevout.IsNull())
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-prevout-null");
}
```

**Null prevout:** `(txid=0x00...00, n=0xFFFFFFFF)`
- Only valid in coinbase transactions
- All other transactions must reference real UTXOs

---

### Stage 2: CheckTxInputs()

**File:** `src/consensus/tx_verify.cpp:164-214`

```cpp
bool Consensus::CheckTxInputs(const CTransaction& tx, TxValidationState& state, 
                               const CCoinsViewCache& inputs, int nSpendHeight, CAmount& txfee)
```

**Purpose:** Context-dependent validation (requires UTXO set).

**Prerequisites:**
- ✅ `CheckTransaction()` must have succeeded
- ✅ `inputs` contains view of UTXO set
- ✅ `nSpendHeight` is height at which tx will be included

**Validation Sequence:**

#### 1. Input Availability Check (lines 167-170)

```cpp
if (!inputs.HaveInputs(tx)) {
    return state.Invalid(TxValidationResult::TX_MISSING_INPUTS, 
                        "bad-txns-inputs-missingorspent",
                        strprintf("%s: inputs missing/spent", __func__));
}
```

**HaveInputs() Logic:**
- Checks that every `txin.prevout` exists in UTXO set
- Verifies coins are not already spent
- Returns `false` if any input is missing or spent

**Note:** Combined with duplicate input check in `CheckTransaction()`, this prevents double-spends.

---

#### 2. Input Value Accumulation with Coinbase Maturity (lines 172-189)

```cpp
CAmount nValueIn = 0;
for (unsigned int i = 0; i < tx.vin.size(); ++i) {
    const COutPoint &prevout = tx.vin[i].prevout;
    const Coin& coin = inputs.AccessCoin(prevout);
    assert(!coin.IsSpent());  // Guaranteed by HaveInputs()
    
    // Coinbase maturity check
    if (coin.IsCoinBase() && nSpendHeight - coin.nHeight < COINBASE_MATURITY) {
        return state.Invalid(TxValidationResult::TX_PREMATURE_SPEND, 
                            "bad-txns-premature-spend-of-coinbase",
                            strprintf("tried to spend coinbase at depth %d", 
                                     nSpendHeight - coin.nHeight));
    }
    
    // Accumulate input value with overflow protection
    nValueIn += coin.out.nValue;
    if (!MoneyRange(coin.out.nValue) || !MoneyRange(nValueIn)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, 
                            "bad-txns-inputvalues-outofrange");
    }
}
```

**Coinbase Maturity:**
- `COINBASE_MATURITY = 100` blocks
- Newly created coins (coinbase/block reward) cannot be spent until 100 confirmations
- Prevents incentive to mine on soon-to-be-invalid chains

**Overflow Protection:**
- Same pattern as output validation
- Check after each addition: `if (!MoneyRange(nValueIn))`

---

#### 3. Fee Validation (lines 196-210)

```cpp
const CAmount value_out = tx.GetValueOut();  // Sum of all outputs

if (nValueIn < value_out) {
    return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-in-belowout",
        strprintf("value in (%s) < value out (%s)", 
                 FormatMoney(nValueIn), FormatMoney(value_out)));
}

// Calculate fee
const CAmount txfee_aux = nValueIn - value_out;
if (!MoneyRange(txfee_aux)) {
    // This should be unreachable given prior checks
    return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-fee-outofrange");
}

txfee = txfee_aux;
return true;
```

**Fee Calculation:**
```
Fee = Sum(inputs) - Sum(outputs)
```

**Constraints:**
- Fee ≥ 0 (enforced by `nValueIn >= value_out` check)
- Fee ≤ `MAX_MONEY` (guaranteed by prior checks)

**Why fee check is "unreachable":**
- Detailed comment at lines 204-208 explains invariants
- `nValueIn ≤ MAX_MONEY` (checked in loop)
- `value_out ≤ MAX_MONEY` (checked in `CheckTransaction`)
- `nValueIn ≥ value_out` (checked above)
- Therefore `0 ≤ fee ≤ MAX_MONEY`

---

## Script Validation Flow

### Script Execution Architecture

```
┌────────────────────────────────────────────────────────────┐
│                 Script Validation System                    │
├────────────────────────────────────────────────────────────┤
│                                                             │
│  CheckInputScripts()  → Top-level script validation        │
│         ↓                                                   │
│  VerifyScript()       → Execute scriptSig + scriptPubKey   │
│         ↓                                                   │
│  EvalScript()         → OpCode interpreter                 │
│         ↓                                                   │
│  Stack Operations     → Push, pop, verify                  │
│         ↓                                                   │
│  CheckSig()           → ECDSA/Schnorr signature check      │
│                                                             │
└────────────────────────────────────────────────────────────┘
```

### Script Types

Bitcoin supports multiple script types through soft forks:

```
┌──────────────┬──────────────┬─────────────────────────────────┐
│ Type         │ Introduced   │ Description                     │
├──────────────┼──────────────┼─────────────────────────────────┤
│ P2PKH        │ Genesis      │ Pay to Public Key Hash          │
│ P2SH         │ BIP16 (2012) │ Pay to Script Hash              │
│ P2WPKH       │ BIP141 (2017)│ Pay to Witness PubKey Hash      │
│ P2WSH        │ BIP141 (2017)│ Pay to Witness Script Hash      │
│ P2TR         │ BIP341 (2021)│ Pay to Taproot (Schnorr)        │
└──────────────┴──────────────┴─────────────────────────────────┘
```

### Signature Verification

**Legacy (P2PKH/P2SH):**
- ECDSA signatures (secp256k1)
- DER-encoded (BIP66 strict DER)
- Signature hash calculation (BIP143 for segwit)

**Segwit (P2WPKH/P2WSH):**
- ECDSA signatures
- Witness data separate from transaction
- Different signature hash algorithm

**Taproot (P2TR):**
- Schnorr signatures (BIP340)
- Key path spending (single signature)
- Script path spending (MAST)

---

## Input Validation Flow

### Complete Input Processing

```
For each transaction input:
    ↓
[1] Fetch referenced output (prevout) from UTXO set
    ↓
[2] Verify output exists and unspent
    ↓
[3] Check coinbase maturity (if applicable)
    ↓
[4] Accumulate input value
    ↓
[5] Prepare script execution context
    ↓
[6] Execute scriptSig (push data to stack)
    ↓
[7] Execute scriptPubKey (verify stack state)
    ↓
[8] For segwit: Execute witness program
    ↓
[9] Verify signature(s) against transaction data
    ↓
[10] Mark input as validated
```

### Signature Cache

**Optimization:** Valid signatures are cached to avoid re-verification.

**Cache Key:**
```cpp
SignatureCacheHasher(tx, scriptCode, flags, ...)
```

**Cache Hit:** Skip expensive ECDSA verification  
**Cache Miss:** Verify signature and add to cache

**Security:** Cache is consensus-critical. Invalid signatures MUST NOT be cached.

---

## Consensus Rules Summary

### Block Rules

| Rule | Limit | Check |
|------|-------|-------|
| **Weight** | 4,000,000 | `block.GetBlockWeight() ≤ MAX_BLOCK_WEIGHT` |
| **Sigops** | 80,000 | `GetBlockSigOpsCost() ≤ MAX_BLOCK_SIGOPS_COST` |
| **Coinbase** | 1 per block | `block.vtx[0]->IsCoinBase() && !block.vtx[i≠0]->IsCoinBase()` |
| **Merkle Root** | Must match | `ComputeMerkleRoot() == block.hashMerkleRoot` |
| **Timestamp** | Within limits | MTP < timestamp < MTP + 2 hours |
| **Difficulty** | Valid target | `nBits` encodes valid target matching PoW |

### Transaction Rules

| Rule | Limit | Check |
|------|-------|-------|
| **Size** | 400,000 WU | `GetSerializeSize() * 4 ≤ MAX_BLOCK_WEIGHT` |
| **Inputs** | ≥ 1 | `!tx.vin.empty()` |
| **Outputs** | ≥ 1 | `!tx.vout.empty()` |
| **Output Value** | ≤ 21M BTC | `nValue ≤ MAX_MONEY` |
| **Total Value** | ≤ 21M BTC | `Sum(outputs) ≤ MAX_MONEY` |
| **Duplicate Inputs** | Not allowed | Set insertion check (CVE-2018-17144) |
| **Fee** | ≥ 0 | `Sum(inputs) ≥ Sum(outputs)` |

### Script Rules

| Rule | Limit | Description |
|------|-------|-------------|
| **Max Script Size** | 10,000 bytes | Prevent bloat |
| **Max Stack Size** | 1,000 elements | Prevent DoS |
| **Max Script Element** | 520 bytes | Individual element limit |
| **Max Ops per Script** | 201 | OpCode limit (legacy) |
| **Max Multisig Keys** | 20 | Keys in OP_CHECKMULTISIG |

### Soft Fork Activation

| BIP | Name | Activation |
|-----|------|------------|
| BIP34 | Block height in coinbase | Height-based |
| BIP65 | OP_CHECKLOCKTIMEVERIFY | Height-based |
| BIP66 | Strict DER signatures | Height-based |
| BIP68/112/113 | Sequence locks (CSV) | Height-based |
| BIP141/143/147 | Segregated Witness | Height-based |
| BIP340/341/342 | Taproot | BIP9 (version bits) |

---

## Validation State Machine

### Block Validation States

```cpp
enum class BlockValidationResult {
    BLOCK_RESULT_UNSET = 0,  // Initial state
    BLOCK_CONSENSUS,         // Invalid by consensus rules (reject permanently)
    BLOCK_RECENT_CONSENSUS_CHANGE,  // Invalid by recent soft fork
    BLOCK_CACHED_INVALID,    // Already known to be invalid
    BLOCK_INVALID_HEADER,    // Invalid header (bad PoW, etc.)
    BLOCK_MUTATED,           // Merkle tree malleability detected
    BLOCK_MISSING_PREV,      // Previous block not found
    BLOCK_INVALID_PREV,      // Previous block is invalid
    BLOCK_TIME_FUTURE,       // Timestamp too far in future
    BLOCK_CHECKPOINT,        // Fails checkpoint verification
};
```

### Transaction Validation States

```cpp
enum class TxValidationResult {
    TX_RESULT_UNSET = 0,        // Initial state
    TX_CONSENSUS,               // Invalid by consensus rules
    TX_RECENT_CONSENSUS_CHANGE, // Invalid by recent soft fork
    TX_INPUTS_NOT_STANDARD,     // Non-standard inputs (policy)
    TX_NOT_STANDARD,            // Non-standard transaction (policy)
    TX_MISSING_INPUTS,          // Inputs missing or spent
    TX_PREMATURE_SPEND,         // Spending immature coinbase
    TX_WITNESS_MUTATED,         // Witness malleated
    TX_WITNESS_STRIPPED,        // Witness stripped
    TX_CONFLICT,                // Conflicts with in-block transaction
    TX_MEMPOOL_POLICY,          // Mempool policy violation
    TX_NO_MEMPOOL,              // Not accepted to mempool (fee too low, etc.)
    TX_RECONSIDERABLE,          // May be valid with different mempool state
    TX_PACKAGE_RECONSIDERABLE,  // May be valid as part of package
};
```

### State Transitions

```
Transaction Submitted
         ↓
    [Validation]
         ↓
    ┌────┴────┐
    ↓         ↓
 Valid    Invalid
    ↓         ↓
[Mempool] [Reject]
    ↓
[Block Inclusion]
    ↓
[UTXO Update]
```

---

## Error Handling

### Validation Errors vs Assertions

**Validation Errors:**
- Expected conditions (invalid blocks, double-spends)
- Return `false` with `state.Invalid(...)`
- Node continues operation

**Assertions:**
- Unexpected conditions (programming bugs, invariant violations)
- **⚠️ DANGER:** Only active in debug builds
- **❌ NEVER use in consensus code** (causes fork)

### Proper Error Handling Pattern

**❌ WRONG:**
```cpp
assert(coin.IsSpent() == false);  // Removed in release builds!
```

**✅ CORRECT:**
```cpp
if (coin.IsSpent()) {
    return state.Invalid(TxValidationResult::TX_MISSING_INPUTS, 
                        "input-already-spent");
}
```

**✅ ACCEPTABLE:**
```cpp
Assume(!coin.IsSpent());  // Logs in release, doesn't crash
if (coin.IsSpent()) {
    return error(...);  // Still handle the condition
}
```

---

## Critical Invariants

### UTXO Set Invariants

1. **No duplicate UTXOs:** Each `(txid, n)` appears at most once
2. **Spent coins removed:** Coins marked spent are deleted from set
3. **Unspent coins available:** All unspent outputs are in UTXO set
4. **Value conservation:** Sum(UTXOs) + Sum(fees) = Original supply + Block rewards

### Block Chain Invariants

1. **Genesis block:** First block is hardcoded (never validated)
2. **Chain continuity:** Each block references previous block hash
3. **Cumulative work:** Chain with most proof-of-work is valid chain
4. **No conflicting transactions:** Within a block, no two transactions spend same output

### Transaction Invariants

1. **Input availability:** Before `UpdateCoins()`, all inputs exist and are unspent
2. **No duplicate inputs:** Within a transaction, each input is unique (CVE-2018-17144)
3. **Value range:** All values in range `[0, MAX_MONEY]`
4. **Non-negative fee:** `Sum(inputs) ≥ Sum(outputs)`

### Validation Ordering Invariants

1. **CheckTransaction before CheckTxInputs:** Context-free before context-dependent
2. **HaveInputs before UpdateCoins:** Verify existence before modification
3. **Script validation after input checks:** Expensive operations last
4. **No witness in CheckBlock:** Witness malleability checked in ContextualCheckBlock

---

## Validation Flow Diagrams

### Complete Block Validation Flowchart

```
                    New Block Received
                            ↓
                 ┌──────────────────┐
                 │ CheckBlockHeader │
                 │  • Proof of Work │
                 └────────┬─────────┘
                          ↓
                      Valid?
                    ↙         ↘
                 No             Yes
                 ↓               ↓
            [Reject]      AcceptBlockHeader
                          • Chain context
                          • Checkpoints
                               ↓
                          CheckBlock
                          • Size limits
                          • Coinbase
                          • Transactions
                          • Merkle root
                               ↓
                     ContextualCheckBlock
                     • Witness commitment
                     • Block weight
                     • Sigop cost
                               ↓
                       ConnectBlock
                       ┌───────┴───────┐
                       ↓               ↓
                CheckTxInputs    CheckInputScripts
                • Input UTXO     • Execute scripts
                • Maturity       • Verify sigs
                • Values         • Witness programs
                       ├───────────────┤
                       ↓
                   UpdateCoins
                   • Mark inputs spent
                   • Add new outputs
                   • Update UTXO set
                       ↓
                   UpdateTip
                   • Activate chain
                   • Update best block
                       ↓
                   Block Accepted
```

### Transaction Validation Flowchart

```
                 Transaction Received
                          ↓
              ┌───────────────────────┐
              │  CheckTransaction     │
              │  • Empty checks       │
              │  • Size limits        │
              │  • Output values      │
              │  • Duplicate inputs   │
              │  • Coinbase format    │
              └───────────┬───────────┘
                          ↓
                      Valid?
                    ↙         ↘
                 No             Yes
                 ↓               ↓
            [Reject]      CheckTxInputs
                          • Input available
                          • Maturity
                          • Input values
                          • Fee > 0
                               ↓
                      CheckInputScripts
                      For each input:
                      ┌─────────────────┐
                      │ • Fetch prevout │
                      │ • Execute script│
                      │ • Verify sig    │
                      │ • Check witness │
                      └────────┬────────┘
                               ↓
                       All inputs valid?
                         ↙         ↘
                      No             Yes
                      ↓               ↓
                 [Reject]       [Accept to Mempool]
                                      ↓
                               [Mine in Block]
                                      ↓
                                 UpdateCoins
                                 • Spend inputs
                                 • Create outputs
                                      ↓
                              Transaction Confirmed
```

---

## Best Practices for Consensus Code

### 1. Never Use Assertions in Consensus Paths

**❌ WRONG:**
```cpp
assert(inputs.HaveInputs(tx));  // Consensus split risk!
```

**✅ CORRECT:**
```cpp
if (!inputs.HaveInputs(tx)) {
    return state.Invalid(TxValidationResult::TX_MISSING_INPUTS, "inputs-missing");
}
```

### 2. Check Overflow After Each Addition

**❌ WRONG:**
```cpp
CAmount total = 0;
for (auto& output : tx.vout) {
    total += output.nValue;  // Could overflow silently!
}
if (!MoneyRange(total)) return false;  // Too late!
```

**✅ CORRECT:**
```cpp
CAmount total = 0;
for (auto& output : tx.vout) {
    total += output.nValue;
    if (!MoneyRange(total)) return false;  // Check immediately!
}
```

### 3. Validate Before Modifying State

**❌ WRONG:**
```cpp
UpdateCoins(view, tx, height);  // Modifies UTXO set
if (!CheckTxInputs(tx, state, view, height, fee)) {
    // Too late! UTXO set already modified!
    return false;
}
```

**✅ CORRECT:**
```cpp
if (!CheckTxInputs(tx, state, view, height, fee)) {
    return false;  // Reject before modifying anything
}
UpdateCoins(view, tx, height);  // Now safe to modify
```

### 4. Separate Context-Free and Context-Dependent Checks

**✅ CORRECT Organization:**

```
Context-Free (src/consensus/tx_check.cpp):
- CheckTransaction()
- No UTXO set needed
- Can be validated in isolation

Context-Dependent (src/consensus/tx_verify.cpp):
- CheckTxInputs()
- Requires UTXO set
- Requires block height
```

### 5. Document Invariants and Assumptions

**✅ GOOD:**
```cpp
// PRECONDITION: All inputs must exist in UTXO set (verified by HaveInputs())
// POSTCONDITION: All inputs marked as spent in view
void UpdateCoins(CCoinsViewCache& view, const CTransaction& tx, int nHeight)
{
    for (const CTxIn& txin : tx.vin) {
        bool is_spent = view.SpendCoin(txin.prevout, &undo);
        if (!is_spent) {
            // Invariant violated! This should never happen if HaveInputs() succeeded.
            LogPrintf("ERROR: UpdateCoins - input missing: %s\n", txin.prevout.ToString());
            return;  // Handle gracefully
        }
    }
    // ... add outputs
}
```

---

## Security Checklist for Consensus Changes

When modifying consensus code, verify:

- [ ] No `assert()` statements in consensus paths
- [ ] All integer additions checked for overflow
- [ ] Validation before state modification
- [ ] Consistent behavior across platforms (no undefined behavior)
- [ ] No floating-point arithmetic
- [ ] No reliance on map/set iteration order (unless documented)
- [ ] Proper error handling for all failure cases
- [ ] Backward compatibility with existing chain
- [ ] Test coverage for edge cases
- [ ] Soft fork if tightening rules, hard fork if relaxing

---

## Appendix: Key Functions Reference

### Block Validation Functions

```cpp
// src/validation.cpp
bool CheckBlockHeader(const CBlockHeader& block, BlockValidationState& state, 
                     const Consensus::Params& consensusParams, bool fCheckPOW = true);

bool CheckBlock(const CBlock& block, BlockValidationState& state, 
               const Consensus::Params& consensusParams, 
               bool fCheckPOW = true, bool fCheckMerkleRoot = true);

bool ContextualCheckBlock(const CBlock& block, BlockValidationState& state, 
                          const CBlockIndex* pindexPrev);

bool ConnectBlock(const CBlock& block, BlockValidationState& state, 
                 CBlockIndex* pindex, CCoinsViewCache& view, 
                 bool fJustCheck = false);
```

### Transaction Validation Functions

```cpp
// src/consensus/tx_check.cpp
bool CheckTransaction(const CTransaction& tx, TxValidationState& state);

// src/consensus/tx_verify.cpp
bool Consensus::CheckTxInputs(const CTransaction& tx, TxValidationState& state, 
                              const CCoinsViewCache& inputs, int nSpendHeight, 
                              CAmount& txfee);

bool CheckInputScripts(const CTransaction& tx, TxValidationState& state, 
                       const CCoinsViewCache& view, unsigned int flags, 
                       bool cacheSigStore, bool cacheFullScriptStore, 
                       PrecomputedTransactionData& txdata, 
                       ValidationCache& validation_cache);
```

### Merkle Tree Functions

```cpp
// src/consensus/merkle.cpp
uint256 ComputeMerkleRoot(std::vector<uint256> hashes, bool* mutated = nullptr);

uint256 BlockMerkleRoot(const CBlock& block, bool* mutated = nullptr);

uint256 BlockWitnessMerkleRoot(const CBlock& block);
```

### Script Validation Functions

```cpp
// src/script/interpreter.cpp
bool EvalScript(std::vector<std::vector<unsigned char>>& stack, 
               const CScript& script, unsigned int flags, 
               const BaseSignatureChecker& checker, 
               SigVersion sigversion, ScriptError* error = nullptr);

bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, 
                 const CScriptWitness* witness, unsigned int flags, 
                 const BaseSignatureChecker& checker, 
                 ScriptError* serror = nullptr);
```

### Utility Functions

```cpp
// src/consensus/amount.h
inline bool MoneyRange(const CAmount& nValue) { 
    return (nValue >= 0 && nValue <= MAX_MONEY); 
}

// src/consensus/tx_verify.cpp
bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime);

std::pair<int, int64_t> CalculateSequenceLocks(const CTransaction &tx, int flags, 
                                                std::vector<int>& prevHeights, 
                                                const CBlockIndex& block);
```

---

## Document History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-02-13 | Initial comprehensive documentation |

---

**End of Document**
