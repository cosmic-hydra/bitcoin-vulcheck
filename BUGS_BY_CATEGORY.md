# Bugs Organized By Category

## Memory Safety - 18 Bugs

Memory safety issues including buffer overflows, use-after-free, dangling pointers, and uninitialized variables.

### Critical (3)
- **BUG-001:** Unsafe Array Access Without Bounds Check (`validation.cpp:3939, 4060`)
- **BUG-006:** Buffer Overflow in LevelDB Logger (`dbwrapper.cpp:60-111`)
- **BUG-007:** Integer Overflow in Vector Deserialization (`serialize.h:818-824`)

### High (9)
- **BUG-010:** Null Pointer Dereference (`validation.cpp:3923, 3939, 4035, 4062`)
- **BUG-015:** GETBLOCKTXN Index Validation Missing (`net_processing.cpp:4306-4325`)
- **BUG-016:** Compact Block Reconstruction - Unvalidated Mempool Reference (`net_processing.cpp:4652-4656`)
- **BUG-025:** Missing Bounds Check on TXNS_RANDOMIZED Index (`txmempool.cpp:287-297`)
- **BUG-028:** Unsafe Memory Operations with Private Keys (`wallet/crypter.cpp:15-39`)
- **BUG-029:** Missing Output Bounds Check in Array Access (`wallet/spend.cpp:528-545`)
- **BUG-035:** Integer Overflow in Vector Allocation (`serialize.h:673`)
- **BUG-038:** Missing Bounds Check in Vector Deserialization (`serialize.h:818, 820`)
- **BUG-041:** Unsafe Pointer Use in CoinEntry (`txdb.cpp:39-45`)
- **BUG-080:** Missing Error Handling in vsnprintf (`dbwrapper.cpp:84`)

### Medium (5)
- **BUG-043:** Use-After-Free in Mempool Acceptance (`validation.cpp:1673, 1683, 1690, 1739`)
- **BUG-049:** Peer State - Null Pointer Dereference Risk (`net_processing.cpp:1096-1104`)
- **BUG-073:** Integer Overflow in prevector Loop Increment (`serialize.h:824`)
- **BUG-075:** Missing Validation in Vector Access (`txdb.cpp:106-113`)

### Low (1)
- **BUG-081:** Assertion Failure Causing Crash (`txdb.cpp:101, 111`)

**Total:** 18 bugs

---

## Integer Overflow/Underflow - 16 Bugs

Issues involving integer arithmetic, overflow, underflow, signed/unsigned mismatches, and type conversions.

### Critical (1)
- **BUG-007:** Integer Overflow in Vector Deserialization (`serialize.h:818-824`)

### High (9)
- **BUG-009:** Integer Underflow in Prune Height Calculation (`validation.cpp:2729-2730`)
- **BUG-020:** Integer Overflow in Cluster Limit Calculation (`txmempool.cpp:169`)
- **BUG-021:** Unsafe Fee Accumulation in Ancestor/Descendant Calculation (`txmempool.cpp:913-918, 927-933`)
- **BUG-022:** Unsigned Integer Underflow in Fee Tracking (`txmempool.cpp:299-300`)
- **BUG-026:** Integer Underflow in Fee-Subtraction Logic (`wallet/spend.cpp:1343-1361`)
- **BUG-035:** Integer Overflow in Vector Allocation (`serialize.h:673`)
- **BUG-036:** Integer Underflow in prevector Deserialization (`serialize.h:821`)
- **BUG-037:** Integer Overflow in Vector Resize (`serialize.h:822`)

### Medium (5)
- **BUG-045:** Integer Overflow in Size Check (`validation.cpp:3993`)
- **BUG-046:** Integer Casting with Potential Data Loss (`validation.cpp:2610, 3814`)
- **BUG-061:** Unchecked Ancestor/Descendant Size Accumulation (`txmempool.cpp:912, 916, 925-926, 931`)
- **BUG-062:** Sequence Number Overflow in TXN Request Tracker (`txrequest.cpp:68, 304, 593`)
- **BUG-067:** Unsigned Integer Wraparound in Output Indexing (`wallet/spend.cpp:1345-1373`)
- **BUG-073:** Integer Overflow in prevector Loop Increment (`serialize.h:824`)
- **BUG-074:** Integer Overflow in Map/Set Deserialization (`serialize.h:909-916`)
- **BUG-076:** Integer Overflow in Batch Size Calculation (`txdb.cpp:136-148`)

### Low (1)
- **BUG-079:** Integer Division in Memory Size Calculation (`dbwrapper.cpp:142-143`)

**Total:** 16 bugs

---

## DoS / Resource Exhaustion - 10 Bugs

Denial of service vulnerabilities through resource exhaustion, unbounded loops, or memory consumption.

### High (5)
- **BUG-012:** ADDRESS Message DoS - Memory Exhaustion (`net_processing.cpp:4019-4023`)
- **BUG-013:** INVENTORY Message DoS - Unbounded Memory Allocation (`net_processing.cpp:4098-4105`)
- **BUG-014:** HEADERS Message Lack of Early Size Validation (`net_processing.cpp:4799-4810`)
- **BUG-024:** Unbounded Transaction Request Tracking (`txrequest.cpp:588-594`)
- **BUG-039:** Unvalidated Input Sizes in String Deserialization (`serialize.h:789-792`)

### Medium (4)
- **BUG-048:** Address Relay Token Bucket - Floating Point Precision (`net_processing.cpp:4031-4035`)
- **BUG-063:** Insufficient Cluster Size Validation (`txmempool.cpp:169-171`)
- **BUG-072:** Missing Resource Limits (`init.cpp:1959-1968, 725-732`)
- **BUG-076:** Integer Overflow in Batch Size Calculation (`txdb.cpp:136-148`)

### Low (1)
- **BUG-082:** SENDCMPCT Parameter - No Validation (`net_processing.cpp:3843-3850`)
- **BUG-083:** Header Synchronization - Potential Infinite Loop (`net_processing.cpp:1472-1489`)

**Total:** 10 bugs

---

## Cryptographic Issues - 14 Bugs

Vulnerabilities in cryptographic operations including weak randomness, timing attacks, side-channels, and key handling.

### Critical (1)
- **BUG-004:** Reproducible Nonce via Test Vectors and Grinding (`key.cpp:214-223`)

### High (3)
- **BUG-017:** Weak Entropy in Signing with Test Vectors (`key.cpp:209-224`)
- **BUG-018:** Stack-Based Secret Data in DER Export (`key.cpp:96-157`)
- **BUG-019:** Extended Key Private Data in Unencrypted Stack Memory (`key.cpp:513-530`)

### Medium (9)
- **BUG-051:** DER Parsing Variable Index Timing Leak (`pubkey.cpp:45-111`)
- **BUG-052:** Non-Constant-Time Private Key Check (`key.cpp:162-168`)
- **BUG-053:** Uninitialized Memory in Signature Parsing (`pubkey.cpp:45-184`)
- **BUG-054:** Insufficient Zeroization of Signing Context (`key.cpp:209-235`)
- **BUG-055:** Timing-Dependent Signature Verification (`key.cpp:237-248`)
- **BUG-056:** Variable-Time BIP32 Key Derivation (`key.cpp:293-310`)
- **BUG-057:** MuSig2 Nonce Invalidation Not Constant-Time (`key.cpp:353-473`)
- **BUG-058:** BIP324 Session Key Not Zeroized (`bip324.cpp:34-71`)
- **BUG-059:** Schnorr Signature Cleanup Only on Failure (`key.cpp:549-563`)
- **BUG-068:** Missing Validation in Crypter Output Parameters (`wallet/crypter.cpp:63-74`)

### Low (1)
- **BUG-078:** RNG State Not Blinded in ECC Context (`key.cpp:572-587`)

**Total:** 14 bugs

---

## Race Conditions / Concurrency - 7 Bugs

Threading issues including race conditions, TOCTOU, and synchronization problems.

### Critical (1)
- **BUG-002:** Race Condition in Witness Commitment Validation (`validation.cpp:2458, 3831-3832, 3916-3948`)

### High (2)
- **BUG-023:** Race Condition in Mempool Transaction Updates (`txmempool.cpp:227-259, 261-304`)
- **BUG-040:** Race Condition in Database State Management (`txdb.cpp:103-114`)

### Medium (4)
- **BUG-047:** Race Condition in Peer Misbehavior Tracking (`net_processing.cpp:1180-1210`)
- **BUG-050:** Bloom Filter - Race Condition (`net_processing.cpp:5037-5045`)
- **BUG-066:** Race Condition in Balance Calculation (`wallet/receive.cpp:245-274`)
- **BUG-069:** Race Condition in blocknotify Callback (`init.cpp:1961-1967`)
- **BUG-070:** Thread Safety Issues with Detached Threads (`init.cpp:1966, 730`)
- **BUG-077:** Unprotected Static Variable in Crash Simulation (`txdb.cpp:141-147`)

**Total:** 7 bugs

---

## Logic Errors - 9 Bugs

Incorrect program logic, off-by-one errors, missing validations, and algorithmic issues.

### High (2)
- **BUG-008:** Missing Bounds Check for Witness Commitment Position (`validation.cpp:3939, 4060`)
- **BUG-011:** Missing Input Validation for Block Size (`validation.cpp:3993`)
- **BUG-027:** Logic Error in Balance Validation (`wallet/spend.cpp:1213-1225`)

### Medium (6)
- **BUG-042:** Off-by-One Error in Block Height Check (`validation.cpp:2204, 2232-2233`)
- **BUG-044:** Assertion-Based Safety (`validation.cpp: throughout`)
- **BUG-064:** Rolling Fee Rate Initialization Bug (`txmempool.cpp:825-826`)
- **BUG-065:** State Machine Invariant Violation in TXREQUEST (`txrequest.cpp:77-93`)

**Total:** 9 bugs

---

## Command Injection / Input Validation - 8 Bugs

Command injection, path traversal, insufficient input validation, and injection attacks.

### Critical (1)
- **BUG-003:** Command Injection via Notification System (`init.cpp:725-732, 251-260, 1959-1968`)

### High (4)
- **BUG-030:** Database Path Traversal in Restore (`wallet/wallet.cpp:477-526`)
- **BUG-031:** startupnotify Command Injection (`init.cpp:725-732`)
- **BUG-032:** shutdownnotify Command Injection (`init.cpp:251-260`)
- **BUG-033:** Insufficient Input Validation in Alert System (`init.cpp:479-480`)
- **BUG-034:** Unused ShellEscape Function (`common/system.cpp:40-47`)

### Medium (2)
- **BUG-071:** Information Disclosure Through Logging (`common/system.cpp:59`)

### Low (1)
- **BUG-084:** Insufficient Descriptor Validation in Import (`wallet/rpc/backup.cpp`)

**Total:** 8 bugs

---

## Wallet / Balance Calculation - 5 Bugs

Issues specific to wallet operations, balance calculations, and fund management.

### Critical (1)
- **BUG-005:** Unvalidated Wallet File Copy in Restore (`wallet/wallet.cpp:525`)

### High (2)
- **BUG-026:** Integer Underflow in Fee-Subtraction Logic (`wallet/spend.cpp:1343-1361`)
- **BUG-029:** Missing Output Bounds Check in Array Access (`wallet/spend.cpp:528-545`)

### Medium (2)
- **BUG-027:** Logic Error in Balance Validation (`wallet/spend.cpp:1213-1225`)
- **BUG-066:** Race Condition in Balance Calculation (`wallet/receive.cpp:245-274`)

**Total:** 5 bugs

---

## Fee Calculation - 5 Bugs

Bugs affecting fee estimation, calculation, and mempool economics.

### High (3)
- **BUG-021:** Unsafe Fee Accumulation in Ancestor/Descendant Calculation (`txmempool.cpp:913-918, 927-933`)
- **BUG-022:** Unsigned Integer Underflow in Fee Tracking (`txmempool.cpp:299-300`)
- **BUG-026:** Integer Underflow in Fee-Subtraction Logic (`wallet/spend.cpp:1343-1361`)

### Medium (2)
- **BUG-060:** Unprotected Rolling Fee Rate Floating-Point Precision (`txmempool.cpp:836`)
- **BUG-064:** Rolling Fee Rate Initialization Bug (`txmempool.cpp:825-826`)

**Total:** 5 bugs

---

## Summary by Category

| Category | Critical | High | Medium | Low | Total |
|----------|----------|------|--------|-----|-------|
| Memory Safety | 3 | 9 | 5 | 1 | 18 |
| Integer Overflow/Underflow | 1 | 8 | 6 | 1 | 16 |
| DoS / Resource Exhaustion | 0 | 5 | 4 | 1 | 10 |
| Cryptographic Issues | 1 | 3 | 9 | 1 | 14 |
| Race Conditions | 1 | 2 | 4 | 0 | 7 |
| Logic Errors | 0 | 3 | 6 | 0 | 9 |
| Command Injection | 1 | 4 | 2 | 1 | 8 |
| Wallet / Balance | 1 | 2 | 2 | 0 | 5 |
| Fee Calculation | 0 | 3 | 2 | 0 | 5 |

**Total Unique Bugs:** 76 (some bugs span multiple categories)

---

## Category Risk Assessment

### Highest Risk Categories

1. **Memory Safety (18 bugs)** - Most prevalent issue type, includes 3 critical bugs
   - Buffer overflows, use-after-free, null pointer dereferences
   - Direct path to code execution and crashes

2. **Integer Overflow/Underflow (16 bugs)** - Second most common
   - Fee calculations, size checks, array indexing
   - Can lead to memory corruption or economic attacks

3. **Cryptographic Issues (14 bugs)** - High severity impact
   - Timing attacks, weak randomness, key exposure
   - Can compromise private keys and transaction security

4. **DoS / Resource Exhaustion (10 bugs)** - Network stability threat
   - Memory exhaustion, unbounded loops
   - Can take down nodes or partition network

### Most Dangerous Bug Types

1. **Consensus-breaking bugs** (BUG-001, BUG-002) - Can split the Bitcoin network
2. **Command injection** (BUG-003, BUG-031, BUG-032) - Remote code execution
3. **Cryptographic weaknesses** (BUG-004, BUG-017) - Private key compromise
4. **Wallet corruption** (BUG-005) - Direct fund loss

### Remediation Priority

**Phase 1 (Immediate):**
- All Critical bugs (7 total)
- Consensus-breaking bugs
- Command injection vectors
- Wallet corruption issues

**Phase 2 (Urgent):**
- High severity memory safety (9 bugs)
- High severity integer issues (8 bugs)
- DoS attack vectors (5 bugs)

**Phase 3 (Important):**
- Medium severity cryptographic issues (9 bugs)
- Race conditions (4 bugs)
- Fee calculation bugs (2 bugs)

**Phase 4 (Monitor):**
- Low severity issues (7 bugs)
- Code quality improvements
- Defense-in-depth enhancements
