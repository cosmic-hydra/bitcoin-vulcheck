# Bugs Organized By File Location

## src/validation.cpp - 12 Bugs

### Critical (2)
- **BUG-001:** Unsafe Array Access Without Bounds Check (Lines: 3939, 4060)
- **BUG-002:** Race Condition in Witness Commitment Validation (Lines: 2458, 3831-3832, 3916-3948)

### High (4)
- **BUG-008:** Missing Bounds Check for Witness Commitment Position (Lines: 3939, 4060)
- **BUG-009:** Integer Underflow in Prune Height Calculation (Lines: 2729-2730)
- **BUG-010:** Null Pointer Dereference (Lines: 3923, 3939, 4035, 4062)
- **BUG-011:** Missing Input Validation for Block Size (Lines: 3993)

### Medium (5)
- **BUG-042:** Off-by-One Error in Block Height Check (Lines: 2204, 2232-2233)
- **BUG-043:** Use-After-Free in Mempool Acceptance (Lines: 1673, 1683, 1690, 1739)
- **BUG-044:** Assertion-Based Safety (Throughout)
- **BUG-045:** Integer Overflow in Size Check (Lines: 3993)
- **BUG-046:** Integer Casting with Potential Data Loss (Lines: 2610, 3814)

### Low (1)
- None specifically identified as low

**Total:** 12 bugs

---

## src/net_processing.cpp - 11 Bugs

### Critical (1)
- None directly critical, but HIGH bugs are severe

### High (5)
- **BUG-012:** ADDRESS Message DoS - Memory Exhaustion (Lines: 4019-4023)
- **BUG-013:** INVENTORY Message DoS - Unbounded Memory Allocation (Lines: 4098-4105)
- **BUG-014:** HEADERS Message Lack of Early Size Validation (Lines: 4799-4810)
- **BUG-015:** GETBLOCKTXN Index Validation Missing (Lines: 4306-4325)
- **BUG-016:** Compact Block Reconstruction - Unvalidated Mempool Reference (Lines: 4652-4656)

### Medium (4)
- **BUG-047:** Race Condition in Peer Misbehavior Tracking (Lines: 1180-1210)
- **BUG-048:** Address Relay Token Bucket - Floating Point Precision (Lines: 4031-4035)
- **BUG-049:** Peer State - Null Pointer Dereference Risk (Lines: 1096-1104)
- **BUG-050:** Bloom Filter - Race Condition (Lines: 5037-5045)

### Low (1)
- **BUG-082:** SENDCMPCT Parameter - No Validation (Lines: 3843-3850)
- **BUG-083:** Header Synchronization - Potential Infinite Loop (Lines: 1472-1489)

**Total:** 11 bugs

---

## src/key.cpp - 10 Bugs

### Critical (1)
- **BUG-004:** Reproducible Nonce via Test Vectors and Grinding (Lines: 214-223)

### High (3)
- **BUG-017:** Weak Entropy in Signing with Test Vectors (Lines: 209-224)
- **BUG-018:** Stack-Based Secret Data in DER Export (Lines: 96-157)
- **BUG-019:** Extended Key Private Data in Unencrypted Stack Memory (Lines: 513-530)

### Medium (5)
- **BUG-052:** Non-Constant-Time Private Key Check (Lines: 162-168)
- **BUG-054:** Insufficient Zeroization of Signing Context (Lines: 209-235)
- **BUG-055:** Timing-Dependent Signature Verification (Lines: 237-248)
- **BUG-056:** Variable-Time BIP32 Key Derivation (Lines: 293-310)
- **BUG-057:** MuSig2 Nonce Invalidation Not Constant-Time (Lines: 353-473)
- **BUG-059:** Schnorr Signature Cleanup Only on Failure (Lines: 549-563)

### Low (1)
- **BUG-078:** RNG State Not Blinded in ECC Context (Lines: 572-587)

**Total:** 10 bugs

---

## src/txmempool.cpp - 9 Bugs

### Critical (0)
- None

### High (4)
- **BUG-020:** Integer Overflow in Cluster Limit Calculation (Lines: 169)
- **BUG-021:** Unsafe Fee Accumulation in Ancestor/Descendant Calculation (Lines: 913-918, 927-933)
- **BUG-022:** Unsigned Integer Underflow in Fee Tracking (Lines: 299-300)
- **BUG-023:** Race Condition in Mempool Transaction Updates (Lines: 227-259, 261-304)
- **BUG-025:** Missing Bounds Check on TXNS_RANDOMIZED Index (Lines: 287-297)

### Medium (4)
- **BUG-060:** Unprotected Rolling Fee Rate Floating-Point Precision (Lines: 836)
- **BUG-061:** Unchecked Ancestor/Descendant Size Accumulation (Lines: 912, 916, 925-926, 931)
- **BUG-063:** Insufficient Cluster Size Validation (Lines: 169-171)
- **BUG-064:** Rolling Fee Rate Initialization Bug (Lines: 825-826)

### Low (0)
- None

**Total:** 9 bugs

---

## src/serialize.h - 8 Bugs

### Critical (1)
- **BUG-007:** Integer Overflow in Vector Deserialization (Lines: 818-824)

### High (4)
- **BUG-035:** Integer Overflow in Vector Allocation (Lines: 673)
- **BUG-036:** Integer Underflow in prevector Deserialization (Lines: 821)
- **BUG-037:** Integer Overflow in Vector Resize (Lines: 822)
- **BUG-038:** Missing Bounds Check in Vector Deserialization (Lines: 818, 820)
- **BUG-039:** Unvalidated Input Sizes in String Deserialization (Lines: 789-792)

### Medium (2)
- **BUG-073:** Integer Overflow in prevector Loop Increment (Lines: 824)
- **BUG-074:** Integer Overflow in Map/Set Deserialization (Lines: 909-916)

### Low (0)
- None

**Total:** 8 bugs

---

## src/wallet/wallet.cpp - 3 Bugs

### Critical (1)
- **BUG-005:** Unvalidated Wallet File Copy in Restore (Lines: 525)

### High (1)
- **BUG-030:** Database Path Traversal in Restore (Lines: 477-526)

### Medium (1)
- **BUG-066:** Race Condition in Balance Calculation (in receive.cpp, related)

### Low (0)
- None

**Total:** 3 bugs (+ related bugs in other wallet files)

---

## src/wallet/spend.cpp - 4 Bugs

### High (2)
- **BUG-026:** Integer Underflow in Fee-Subtraction Logic (Lines: 1343-1361)
- **BUG-029:** Missing Output Bounds Check in Array Access (Lines: 528-545)

### Medium (2)
- **BUG-027:** Logic Error in Balance Validation (Lines: 1213-1225)
- **BUG-067:** Unsigned Integer Wraparound in Output Indexing (Lines: 1345-1373)

**Total:** 4 bugs

---

## src/wallet/crypter.cpp - 2 Bugs

### High (1)
- **BUG-028:** Unsafe Memory Operations with Private Keys (Lines: 15-39)

### Medium (1)
- **BUG-068:** Missing Validation in Crypter Output Parameters (Lines: 63-74)

**Total:** 2 bugs

---

## src/init.cpp - 6 Bugs

### Critical (1)
- **BUG-003:** Command Injection via Notification System (Lines: 725-732, 251-260, 1959-1968)

### High (3)
- **BUG-031:** startupnotify Command Injection (Lines: 725-732)
- **BUG-032:** shutdownnotify Command Injection (Lines: 251-260)
- **BUG-033:** Insufficient Input Validation in Alert System (Lines: 479-480)

### Medium (2)
- **BUG-069:** Race Condition in blocknotify Callback (Lines: 1961-1967)
- **BUG-070:** Thread Safety Issues with Detached Threads (Lines: 1966, 730)
- **BUG-072:** Missing Resource Limits (Lines: 1959-1968, 725-732)

**Total:** 6 bugs

---

## src/dbwrapper.cpp - 3 Bugs

### Critical (1)
- **BUG-006:** Buffer Overflow in LevelDB Logger (Lines: 60-111)

### High (1)
- **BUG-080:** Missing Error Handling in vsnprintf (Lines: 84)

### Low (1)
- **BUG-079:** Integer Division in Memory Size Calculation (Lines: 142-143)

**Total:** 3 bugs

---

## src/txdb.cpp - 6 Bugs

### High (2)
- **BUG-040:** Race Condition in Database State Management (Lines: 103-114)
- **BUG-041:** Unsafe Pointer Use in CoinEntry (Lines: 39-45)

### Medium (3)
- **BUG-075:** Missing Validation in Vector Access (Lines: 106-113)
- **BUG-076:** Integer Overflow in Batch Size Calculation (Lines: 136-148)
- **BUG-077:** Unprotected Static Variable in Crash Simulation (Lines: 141-147)

### Low (1)
- **BUG-081:** Assertion Failure Causing Crash (Lines: 101, 111)

**Total:** 6 bugs

---

## src/txrequest.cpp - 3 Bugs

### High (1)
- **BUG-024:** Unbounded Transaction Request Tracking (Lines: 588-594)

### Medium (2)
- **BUG-062:** Sequence Number Overflow in TXN Request Tracker (Lines: 68, 304, 593)
- **BUG-065:** State Machine Invariant Violation in TXREQUEST (Lines: 77-93)

**Total:** 3 bugs

---

## src/pubkey.cpp - 2 Bugs

### Medium (2)
- **BUG-051:** DER Parsing Variable Index Timing Leak (Lines: 45-111)
- **BUG-053:** Uninitialized Memory in Signature Parsing (Lines: 45-184)

**Total:** 2 bugs

---

## src/bip324.cpp - 1 Bug

### Medium (1)
- **BUG-058:** BIP324 Session Key Not Zeroized (Lines: 34-71)

**Total:** 1 bug

---

## src/common/system.cpp - 2 Bugs

### High (1)
- **BUG-034:** Unused ShellEscape Function (Lines: 40-47)

### Medium (1)
- **BUG-071:** Information Disclosure Through Logging (Lines: 59)

**Total:** 2 bugs

---

## src/wallet/receive.cpp - 1 Bug

### Medium (1)
- **BUG-066:** Race Condition in Balance Calculation (Lines: 245-274)

**Total:** 1 bug

---

## src/wallet/rpc/backup.cpp - 1 Bug

### Low (1)
- **BUG-084:** Insufficient Descriptor Validation in Import

**Total:** 1 bug

---

## Summary by File

| File | Critical | High | Medium | Low | Total |
|------|----------|------|--------|-----|-------|
| validation.cpp | 2 | 4 | 5 | 0 | 11 |
| net_processing.cpp | 0 | 5 | 4 | 2 | 11 |
| key.cpp | 1 | 3 | 5 | 1 | 10 |
| txmempool.cpp | 0 | 4 | 4 | 0 | 8 |
| serialize.h | 1 | 4 | 2 | 0 | 7 |
| init.cpp | 1 | 3 | 2 | 0 | 6 |
| txdb.cpp | 0 | 2 | 3 | 1 | 6 |
| spend.cpp | 0 | 2 | 2 | 0 | 4 |
| wallet.cpp | 1 | 1 | 1 | 0 | 3 |
| dbwrapper.cpp | 1 | 1 | 0 | 1 | 3 |
| txrequest.cpp | 0 | 1 | 2 | 0 | 3 |
| crypter.cpp | 0 | 1 | 1 | 0 | 2 |
| pubkey.cpp | 0 | 0 | 2 | 0 | 2 |
| system.cpp | 0 | 1 | 1 | 0 | 2 |
| receive.cpp | 0 | 0 | 1 | 0 | 1 |
| bip324.cpp | 0 | 0 | 1 | 0 | 1 |
| rpc/backup.cpp | 0 | 0 | 0 | 1 | 1 |
| **TOTAL** | **7** | **32** | **36** | **6** | **81** |

## Files Requiring Immediate Attention

1. **validation.cpp** - 11 bugs, 2 critical (consensus-breaking)
2. **net_processing.cpp** - 11 bugs, 5 high (DoS vectors)
3. **key.cpp** - 10 bugs, 1 critical (crypto weakness)
4. **txmempool.cpp** - 8 bugs (fee/mempool issues)
5. **serialize.h** - 7 bugs, 1 critical (deserialization)
6. **init.cpp** - 6 bugs, 1 critical (command injection)
7. **txdb.cpp** - 6 bugs (database corruption)

These 7 files contain 59 of the 81 total bugs (72.8%).
