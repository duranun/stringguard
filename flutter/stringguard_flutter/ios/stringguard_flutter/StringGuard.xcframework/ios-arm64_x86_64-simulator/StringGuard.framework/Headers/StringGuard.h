#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

@class NSData, StringGuardAttestationOutcomeCertChain, StringGuardAttestationOutcomeUnsupported, StringGuardAttestationStatement, StringGuardDetectionCategory, StringGuardDetections, StringGuardDeviceProof, StringGuardDeviceSignals, StringGuardGuard, StringGuardGuardReport, StringGuardKotlinArray<T>, StringGuardKotlinByteArray, StringGuardKotlinByteIterator, StringGuardKotlinEnum<E>, StringGuardKotlinEnumCompanion, StringGuardKotlinException, StringGuardKotlinIllegalStateException, StringGuardKotlinRuntimeException, StringGuardKotlinThrowable, StringGuardRequestSigning, StringGuardSignedRequestEnvelope, StringGuardStringGuardKit;

@protocol StringGuardAttestationOutcome, StringGuardKotlinComparable, StringGuardKotlinIterator;

NS_ASSUME_NONNULL_BEGIN
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wincompatible-property-type"
#pragma clang diagnostic ignored "-Wnullability"

#pragma push_macro("_Nullable_result")
#if !__has_feature(nullability_nullable_result)
#undef _Nullable_result
#define _Nullable_result _Nullable
#endif

__attribute__((swift_name("KotlinBase")))
@interface StringGuardBase : NSObject
- (instancetype)init __attribute__((unavailable));
+ (instancetype)new __attribute__((unavailable));
+ (void)initialize __attribute__((objc_requires_super));
@end

@interface StringGuardBase (StringGuardBaseCopying) <NSCopying>
@end

__attribute__((swift_name("KotlinMutableSet")))
@interface StringGuardMutableSet<ObjectType> : NSMutableSet<ObjectType>
@end

__attribute__((swift_name("KotlinMutableDictionary")))
@interface StringGuardMutableDictionary<KeyType, ObjectType> : NSMutableDictionary<KeyType, ObjectType>
@end

@interface NSError (NSErrorStringGuardKotlinException)
@property (readonly) id _Nullable kotlinException;
@end

__attribute__((swift_name("KotlinNumber")))
@interface StringGuardNumber : NSNumber
- (instancetype)initWithChar:(char)value __attribute__((unavailable));
- (instancetype)initWithUnsignedChar:(unsigned char)value __attribute__((unavailable));
- (instancetype)initWithShort:(short)value __attribute__((unavailable));
- (instancetype)initWithUnsignedShort:(unsigned short)value __attribute__((unavailable));
- (instancetype)initWithInt:(int)value __attribute__((unavailable));
- (instancetype)initWithUnsignedInt:(unsigned int)value __attribute__((unavailable));
- (instancetype)initWithLong:(long)value __attribute__((unavailable));
- (instancetype)initWithUnsignedLong:(unsigned long)value __attribute__((unavailable));
- (instancetype)initWithLongLong:(long long)value __attribute__((unavailable));
- (instancetype)initWithUnsignedLongLong:(unsigned long long)value __attribute__((unavailable));
- (instancetype)initWithFloat:(float)value __attribute__((unavailable));
- (instancetype)initWithDouble:(double)value __attribute__((unavailable));
- (instancetype)initWithBool:(BOOL)value __attribute__((unavailable));
- (instancetype)initWithInteger:(NSInteger)value __attribute__((unavailable));
- (instancetype)initWithUnsignedInteger:(NSUInteger)value __attribute__((unavailable));
+ (instancetype)numberWithChar:(char)value __attribute__((unavailable));
+ (instancetype)numberWithUnsignedChar:(unsigned char)value __attribute__((unavailable));
+ (instancetype)numberWithShort:(short)value __attribute__((unavailable));
+ (instancetype)numberWithUnsignedShort:(unsigned short)value __attribute__((unavailable));
+ (instancetype)numberWithInt:(int)value __attribute__((unavailable));
+ (instancetype)numberWithUnsignedInt:(unsigned int)value __attribute__((unavailable));
+ (instancetype)numberWithLong:(long)value __attribute__((unavailable));
+ (instancetype)numberWithUnsignedLong:(unsigned long)value __attribute__((unavailable));
+ (instancetype)numberWithLongLong:(long long)value __attribute__((unavailable));
+ (instancetype)numberWithUnsignedLongLong:(unsigned long long)value __attribute__((unavailable));
+ (instancetype)numberWithFloat:(float)value __attribute__((unavailable));
+ (instancetype)numberWithDouble:(double)value __attribute__((unavailable));
+ (instancetype)numberWithBool:(BOOL)value __attribute__((unavailable));
+ (instancetype)numberWithInteger:(NSInteger)value __attribute__((unavailable));
+ (instancetype)numberWithUnsignedInteger:(NSUInteger)value __attribute__((unavailable));
@end

__attribute__((swift_name("KotlinByte")))
@interface StringGuardByte : StringGuardNumber
- (instancetype)initWithChar:(char)value;
+ (instancetype)numberWithChar:(char)value;
@end

__attribute__((swift_name("KotlinUByte")))
@interface StringGuardUByte : StringGuardNumber
- (instancetype)initWithUnsignedChar:(unsigned char)value;
+ (instancetype)numberWithUnsignedChar:(unsigned char)value;
@end

__attribute__((swift_name("KotlinShort")))
@interface StringGuardShort : StringGuardNumber
- (instancetype)initWithShort:(short)value;
+ (instancetype)numberWithShort:(short)value;
@end

__attribute__((swift_name("KotlinUShort")))
@interface StringGuardUShort : StringGuardNumber
- (instancetype)initWithUnsignedShort:(unsigned short)value;
+ (instancetype)numberWithUnsignedShort:(unsigned short)value;
@end

__attribute__((swift_name("KotlinInt")))
@interface StringGuardInt : StringGuardNumber
- (instancetype)initWithInt:(int)value;
+ (instancetype)numberWithInt:(int)value;
@end

__attribute__((swift_name("KotlinUInt")))
@interface StringGuardUInt : StringGuardNumber
- (instancetype)initWithUnsignedInt:(unsigned int)value;
+ (instancetype)numberWithUnsignedInt:(unsigned int)value;
@end

__attribute__((swift_name("KotlinLong")))
@interface StringGuardLong : StringGuardNumber
- (instancetype)initWithLongLong:(long long)value;
+ (instancetype)numberWithLongLong:(long long)value;
@end

__attribute__((swift_name("KotlinULong")))
@interface StringGuardULong : StringGuardNumber
- (instancetype)initWithUnsignedLongLong:(unsigned long long)value;
+ (instancetype)numberWithUnsignedLongLong:(unsigned long long)value;
@end

__attribute__((swift_name("KotlinFloat")))
@interface StringGuardFloat : StringGuardNumber
- (instancetype)initWithFloat:(float)value;
+ (instancetype)numberWithFloat:(float)value;
@end

__attribute__((swift_name("KotlinDouble")))
@interface StringGuardDouble : StringGuardNumber
- (instancetype)initWithDouble:(double)value;
+ (instancetype)numberWithDouble:(double)value;
@end

__attribute__((swift_name("KotlinBoolean")))
@interface StringGuardBoolean : StringGuardNumber
- (instancetype)initWithBool:(BOOL)value;
+ (instancetype)numberWithBool:(BOOL)value;
@end

__attribute__((swift_name("AttestationOutcome")))
@protocol StringGuardAttestationOutcome
@required
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("AttestationOutcomeAppAttest")))
@interface StringGuardAttestationOutcomeAppAttest : StringGuardBase <StringGuardAttestationOutcome>
- (instancetype)initWithKeyId:(NSString *)keyId attestationObject:(StringGuardKotlinByteArray *)attestationObject __attribute__((swift_name("init(keyId:attestationObject:)"))) __attribute__((objc_designated_initializer));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) StringGuardKotlinByteArray *attestationObject __attribute__((swift_name("attestationObject")));
@property (readonly) NSString *keyId __attribute__((swift_name("keyId")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("AttestationOutcomeCertChain")))
@interface StringGuardAttestationOutcomeCertChain : StringGuardBase <StringGuardAttestationOutcome>
- (instancetype)initWithDerChain:(NSArray<StringGuardKotlinByteArray *> *)derChain strongBox:(BOOL)strongBox __attribute__((swift_name("init(derChain:strongBox:)"))) __attribute__((objc_designated_initializer));
- (StringGuardAttestationOutcomeCertChain *)doCopyDerChain:(NSArray<StringGuardKotlinByteArray *> *)derChain strongBox:(BOOL)strongBox __attribute__((swift_name("doCopy(derChain:strongBox:)")));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) NSArray<StringGuardKotlinByteArray *> *derChain __attribute__((swift_name("derChain")));
@property (readonly) BOOL strongBox __attribute__((swift_name("strongBox")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("AttestationOutcomeUnsupported")))
@interface StringGuardAttestationOutcomeUnsupported : StringGuardBase <StringGuardAttestationOutcome>
- (instancetype)initWithReason:(NSString *)reason __attribute__((swift_name("init(reason:)"))) __attribute__((objc_designated_initializer));
- (StringGuardAttestationOutcomeUnsupported *)doCopyReason:(NSString *)reason __attribute__((swift_name("doCopy(reason:)")));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) NSString *reason __attribute__((swift_name("reason")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("AttestationStatement")))
@interface StringGuardAttestationStatement : StringGuardBase
- (instancetype)initWithNonce:(StringGuardKotlinByteArray *)nonce outcome:(id<StringGuardAttestationOutcome>)outcome signals:(StringGuardDeviceSignals * _Nullable)signals createdAtEpochMs:(int64_t)createdAtEpochMs __attribute__((swift_name("init(nonce:outcome:signals:createdAtEpochMs:)"))) __attribute__((objc_designated_initializer));
- (StringGuardAttestationStatement *)doCopyNonce:(StringGuardKotlinByteArray *)nonce outcome:(id<StringGuardAttestationOutcome>)outcome signals:(StringGuardDeviceSignals * _Nullable)signals createdAtEpochMs:(int64_t)createdAtEpochMs __attribute__((swift_name("doCopy(nonce:outcome:signals:createdAtEpochMs:)")));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) int64_t createdAtEpochMs __attribute__((swift_name("createdAtEpochMs")));
@property (readonly) StringGuardKotlinByteArray *nonce __attribute__((swift_name("nonce")));
@property (readonly) id<StringGuardAttestationOutcome> outcome __attribute__((swift_name("outcome")));
@property (readonly) StringGuardDeviceSignals * _Nullable signals __attribute__((swift_name("signals")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("DeviceProof")))
@interface StringGuardDeviceProof : StringGuardBase
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
+ (instancetype)deviceProof __attribute__((swift_name("init()")));
@property (class, readonly, getter=shared) StringGuardDeviceProof *shared __attribute__((swift_name("shared")));
- (StringGuardKotlinByteArray *)canonicalBytesAudience:(NSString *)audience nonce:(StringGuardKotlinByteArray *)nonce __attribute__((swift_name("canonicalBytes(audience:nonce:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("DeviceSignals")))
@interface StringGuardDeviceSignals : StringGuardBase
- (instancetype)initWithDebuggerAttached:(BOOL)debuggerAttached rootDetected:(BOOL)rootDetected hookingDetected:(BOOL)hookingDetected __attribute__((swift_name("init(debuggerAttached:rootDetected:hookingDetected:)"))) __attribute__((objc_designated_initializer));
- (StringGuardDeviceSignals *)doCopyDebuggerAttached:(BOOL)debuggerAttached rootDetected:(BOOL)rootDetected hookingDetected:(BOOL)hookingDetected __attribute__((swift_name("doCopy(debuggerAttached:rootDetected:hookingDetected:)")));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) BOOL debuggerAttached __attribute__((swift_name("debuggerAttached")));
@property (readonly) BOOL hookingDetected __attribute__((swift_name("hookingDetected")));
@property (readonly) BOOL rootDetected __attribute__((swift_name("rootDetected")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("RequestSigning")))
@interface StringGuardRequestSigning : StringGuardBase
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
+ (instancetype)requestSigning __attribute__((swift_name("init()")));
@property (class, readonly, getter=shared) StringGuardRequestSigning *shared __attribute__((swift_name("shared")));
- (StringGuardKotlinByteArray *)canonicalBytesSessionToken:(NSString *)sessionToken method:(NSString *)method path:(NSString *)path bodySha256:(StringGuardKotlinByteArray *)bodySha256 timestampMs:(int64_t)timestampMs requestNonce:(StringGuardKotlinByteArray *)requestNonce __attribute__((swift_name("canonicalBytes(sessionToken:method:path:bodySha256:timestampMs:requestNonce:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("SignedRequestEnvelope")))
@interface StringGuardSignedRequestEnvelope : StringGuardBase
- (instancetype)initWithSessionToken:(NSString *)sessionToken method:(NSString *)method path:(NSString *)path bodySha256:(StringGuardKotlinByteArray *)bodySha256 timestampMs:(int64_t)timestampMs requestNonce:(StringGuardKotlinByteArray *)requestNonce signature:(StringGuardKotlinByteArray *)signature __attribute__((swift_name("init(sessionToken:method:path:bodySha256:timestampMs:requestNonce:signature:)"))) __attribute__((objc_designated_initializer));
@property (readonly) StringGuardKotlinByteArray *bodySha256 __attribute__((swift_name("bodySha256")));
@property (readonly) NSString *method __attribute__((swift_name("method")));
@property (readonly) NSString *path __attribute__((swift_name("path")));
@property (readonly) StringGuardKotlinByteArray *requestNonce __attribute__((swift_name("requestNonce")));
@property (readonly) NSString *sessionToken __attribute__((swift_name("sessionToken")));
@property (readonly) StringGuardKotlinByteArray *signature __attribute__((swift_name("signature")));
@property (readonly) int64_t timestampMs __attribute__((swift_name("timestampMs")));
@end

__attribute__((swift_name("KotlinComparable")))
@protocol StringGuardKotlinComparable
@required
- (int32_t)compareToOther:(id _Nullable)other __attribute__((swift_name("compareTo(other:)")));
@end

__attribute__((swift_name("KotlinEnum")))
@interface StringGuardKotlinEnum<E> : StringGuardBase <StringGuardKotlinComparable>
- (instancetype)initWithName:(NSString *)name ordinal:(int32_t)ordinal __attribute__((swift_name("init(name:ordinal:)"))) __attribute__((objc_designated_initializer));
@property (class, readonly, getter=companion) StringGuardKotlinEnumCompanion *companion __attribute__((swift_name("companion")));
- (int32_t)compareToOther:(E)other __attribute__((swift_name("compareTo(other:)")));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) NSString *name __attribute__((swift_name("name")));
@property (readonly) int32_t ordinal __attribute__((swift_name("ordinal")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("DetectionCategory")))
@interface StringGuardDetectionCategory : StringGuardKotlinEnum<StringGuardDetectionCategory *>
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
- (instancetype)initWithName:(NSString *)name ordinal:(int32_t)ordinal __attribute__((swift_name("init(name:ordinal:)"))) __attribute__((objc_designated_initializer)) __attribute__((unavailable));
@property (class, readonly) StringGuardDetectionCategory *debugger __attribute__((swift_name("debugger")));
@property (class, readonly) StringGuardDetectionCategory *hookingFramework __attribute__((swift_name("hookingFramework")));
@property (class, readonly) StringGuardDetectionCategory *rootOrJailbreak __attribute__((swift_name("rootOrJailbreak")));
+ (StringGuardKotlinArray<StringGuardDetectionCategory *> *)values __attribute__((swift_name("values()")));
@property (class, readonly) NSArray<StringGuardDetectionCategory *> *entries __attribute__((swift_name("entries")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("Detections")))
@interface StringGuardDetections : StringGuardBase
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
+ (instancetype)detections __attribute__((swift_name("init()")));
@property (class, readonly, getter=shared) StringGuardDetections *shared __attribute__((swift_name("shared")));
- (NSArray<StringGuardDetectionCategory *> *)all __attribute__((swift_name("all()")));
- (void)reset __attribute__((swift_name("reset()")));
- (BOOL)seenCategory:(StringGuardDetectionCategory *)category __attribute__((swift_name("seen(category:)")));
@property void (^ _Nullable onDetection)(StringGuardDetectionCategory *) __attribute__((swift_name("onDetection")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("Guard")))
@interface StringGuardGuard : StringGuardBase
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
+ (instancetype)guard __attribute__((swift_name("init()")));
@property (class, readonly, getter=shared) StringGuardGuard *shared __attribute__((swift_name("shared")));
- (id _Nullable)protectOnDetected:(id _Nullable (^)(void))onDetected block:(id _Nullable (^)(void))block __attribute__((swift_name("protect(onDetected:block:)")));
- (StringGuardGuardReport *)report __attribute__((swift_name("report()")));
@property BOOL rootIsSuspicious __attribute__((swift_name("rootIsSuspicious")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("GuardReport")))
@interface StringGuardGuardReport : StringGuardBase
- (instancetype)initWithDebuggerAttached:(BOOL)debuggerAttached rootDetected:(BOOL)rootDetected hookingDetected:(BOOL)hookingDetected rootCountsAsSuspicious:(BOOL)rootCountsAsSuspicious __attribute__((swift_name("init(debuggerAttached:rootDetected:hookingDetected:rootCountsAsSuspicious:)"))) __attribute__((objc_designated_initializer));
- (StringGuardGuardReport *)doCopyDebuggerAttached:(BOOL)debuggerAttached rootDetected:(BOOL)rootDetected hookingDetected:(BOOL)hookingDetected rootCountsAsSuspicious:(BOOL)rootCountsAsSuspicious __attribute__((swift_name("doCopy(debuggerAttached:rootDetected:hookingDetected:rootCountsAsSuspicious:)")));
- (BOOL)isEqual:(id _Nullable)other __attribute__((swift_name("isEqual(_:)")));
- (NSUInteger)hash __attribute__((swift_name("hash()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) BOOL debuggerAttached __attribute__((swift_name("debuggerAttached")));
@property (readonly) BOOL hookingDetected __attribute__((swift_name("hookingDetected")));
@property (readonly) BOOL rootCountsAsSuspicious __attribute__((swift_name("rootCountsAsSuspicious")));
@property (readonly) BOOL rootDetected __attribute__((swift_name("rootDetected")));
@property (readonly) BOOL suspicious __attribute__((swift_name("suspicious")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("StringGuardKit")))
@interface StringGuardStringGuardKit : StringGuardBase
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
+ (instancetype)stringGuardKit __attribute__((swift_name("init()")));
@property (class, readonly, getter=shared) StringGuardStringGuardKit *shared __attribute__((swift_name("shared")));

/**
 * @note This method converts instances of CancellationException to errors.
 * Other uncaught Kotlin exceptions are fatal.
*/
- (void)assertionKeyId:(NSString *)keyId audience:(NSString *)audience nonce:(NSData *)nonce completionHandler:(void (^)(NSData * _Nullable_result, NSError * _Nullable))completionHandler __attribute__((swift_name("assertion(keyId:audience:nonce:completionHandler:)")));

/**
 * @note This method converts instances of CancellationException to errors.
 * Other uncaught Kotlin exceptions are fatal.
*/
- (void)attestNonce:(NSData *)nonce completionHandler:(void (^)(StringGuardAttestationStatement * _Nullable, NSError * _Nullable))completionHandler __attribute__((swift_name("attest(nonce:completionHandler:)")));
- (NSData * _Nullable)attestationObjectOutcome:(id<StringGuardAttestationOutcome>)outcome __attribute__((swift_name("attestationObject(outcome:)")));
- (NSString * _Nullable)keyIdOutcome:(id<StringGuardAttestationOutcome>)outcome __attribute__((swift_name("keyId(outcome:)")));
- (StringGuardDeviceSignals *)signals __attribute__((swift_name("signals()")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("AppAssertion_appleKt")))
@interface StringGuardAppAssertion_appleKt : StringGuardBase

/**
 * @note This method converts instances of CancellationException to errors.
 * Other uncaught Kotlin exceptions are fatal.
*/
+ (void)appleDeviceAssertionKeyId:(NSString *)keyId audience:(NSString *)audience nonce:(StringGuardKotlinByteArray *)nonce completionHandler:(void (^)(StringGuardKotlinByteArray * _Nullable_result, NSError * _Nullable))completionHandler __attribute__((swift_name("appleDeviceAssertion(keyId:audience:nonce:completionHandler:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("Attestation_appleKt")))
@interface StringGuardAttestation_appleKt : StringGuardBase
+ (id<StringGuardAttestationOutcome>)platformAttestNonce:(StringGuardKotlinByteArray *)nonce __attribute__((swift_name("platformAttest(nonce:)")));

/**
 * @note This method converts instances of CancellationException to errors.
 * Other uncaught Kotlin exceptions are fatal.
*/
+ (void)platformAttestSuspendingNonce:(StringGuardKotlinByteArray *)nonce completionHandler:(void (^)(id<StringGuardAttestationOutcome> _Nullable, NSError * _Nullable))completionHandler __attribute__((swift_name("platformAttestSuspending(nonce:completionHandler:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("AttestationKt")))
@interface StringGuardAttestationKt : StringGuardBase
+ (StringGuardAttestationStatement *)buildAttestationStatementNonce:(StringGuardKotlinByteArray *)nonce signals:(StringGuardDeviceSignals * _Nullable)signals nowEpochMs:(int64_t)nowEpochMs __attribute__((swift_name("buildAttestationStatement(nonce:signals:nowEpochMs:)")));

/**
 * @note This method converts instances of CancellationException to errors.
 * Other uncaught Kotlin exceptions are fatal.
*/
+ (void)buildAttestationStatementSuspendingNonce:(StringGuardKotlinByteArray *)nonce signals:(StringGuardDeviceSignals * _Nullable)signals nowEpochMs:(int64_t)nowEpochMs completionHandler:(void (^)(StringGuardAttestationStatement * _Nullable, NSError * _Nullable))completionHandler __attribute__((swift_name("buildAttestationStatementSuspending(nonce:signals:nowEpochMs:completionHandler:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("GuardKt")))
@interface StringGuardGuardKt : StringGuardBase
+ (NSString *)degradeOriginal:(NSString *)original __attribute__((swift_name("degrade(original:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("RequestSigning_appleKt")))
@interface StringGuardRequestSigning_appleKt : StringGuardBase
+ (id<StringGuardAttestationOutcome>)platformAttestPersistentNonce:(StringGuardKotlinByteArray *)nonce alias:(NSString *)alias __attribute__((swift_name("platformAttestPersistent(nonce:alias:)")));
+ (void)platformDeleteKeyAlias:(NSString *)alias __attribute__((swift_name("platformDeleteKey(alias:)")));
+ (StringGuardKotlinByteArray * _Nullable)platformSignAlias:(NSString *)alias payload:(StringGuardKotlinByteArray *)payload __attribute__((swift_name("platformSign(alias:payload:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("RequestSigningKt")))
@interface StringGuardRequestSigningKt : StringGuardBase
+ (StringGuardSignedRequestEnvelope * _Nullable)signRequestAlias:(NSString *)alias sessionToken:(NSString *)sessionToken method:(NSString *)method path:(NSString *)path bodySha256:(StringGuardKotlinByteArray *)bodySha256 timestampMs:(int64_t)timestampMs requestNonce:(StringGuardKotlinByteArray *)requestNonce __attribute__((swift_name("signRequest(alias:sessionToken:method:path:bodySha256:timestampMs:requestNonce:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("TenantBindingKt")))
@interface StringGuardTenantBindingKt : StringGuardBase
+ (NSString *)deviceKeyAliasTenantId:(NSString *)tenantId __attribute__((swift_name("deviceKeyAlias(tenantId:)")));
+ (StringGuardKotlinByteArray * _Nullable)signDeviceProofAlias:(NSString *)alias audience:(NSString *)audience nonce:(StringGuardKotlinByteArray *)nonce __attribute__((swift_name("signDeviceProof(alias:audience:nonce:)")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("KotlinByteArray")))
@interface StringGuardKotlinByteArray : StringGuardBase
+ (instancetype)arrayWithSize:(int32_t)size __attribute__((swift_name("init(size:)")));
+ (instancetype)arrayWithSize:(int32_t)size init:(StringGuardByte *(^)(StringGuardInt *))init __attribute__((swift_name("init(size:init:)")));
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
- (int8_t)getIndex:(int32_t)index __attribute__((swift_name("get(index:)")));
- (StringGuardKotlinByteIterator *)iterator __attribute__((swift_name("iterator()")));
- (void)setIndex:(int32_t)index value:(int8_t)value __attribute__((swift_name("set(index:value:)")));
@property (readonly) int32_t size __attribute__((swift_name("size")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("KotlinEnumCompanion")))
@interface StringGuardKotlinEnumCompanion : StringGuardBase
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
+ (instancetype)companion __attribute__((swift_name("init()")));
@property (class, readonly, getter=shared) StringGuardKotlinEnumCompanion *shared __attribute__((swift_name("shared")));
@end

__attribute__((objc_subclassing_restricted))
__attribute__((swift_name("KotlinArray")))
@interface StringGuardKotlinArray<T> : StringGuardBase
+ (instancetype)arrayWithSize:(int32_t)size init:(T _Nullable (^)(StringGuardInt *))init __attribute__((swift_name("init(size:init:)")));
+ (instancetype)alloc __attribute__((unavailable));
+ (instancetype)allocWithZone:(struct _NSZone *)zone __attribute__((unavailable));
- (T _Nullable)getIndex:(int32_t)index __attribute__((swift_name("get(index:)")));
- (id<StringGuardKotlinIterator>)iterator __attribute__((swift_name("iterator()")));
- (void)setIndex:(int32_t)index value:(T _Nullable)value __attribute__((swift_name("set(index:value:)")));
@property (readonly) int32_t size __attribute__((swift_name("size")));
@end

__attribute__((swift_name("KotlinThrowable")))
@interface StringGuardKotlinThrowable : StringGuardBase
- (instancetype)init __attribute__((swift_name("init()"))) __attribute__((objc_designated_initializer));
+ (instancetype)new __attribute__((availability(swift, unavailable, message="use object initializers instead")));
- (instancetype)initWithMessage:(NSString * _Nullable)message __attribute__((swift_name("init(message:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithCause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(cause:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithMessage:(NSString * _Nullable)message cause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(message:cause:)"))) __attribute__((objc_designated_initializer));

/**
 * @note annotations
 *   kotlin.experimental.ExperimentalNativeApi
*/
- (StringGuardKotlinArray<NSString *> *)getStackTrace __attribute__((swift_name("getStackTrace()")));
- (void)printStackTrace __attribute__((swift_name("printStackTrace()")));
- (NSString *)description __attribute__((swift_name("description()")));
@property (readonly) StringGuardKotlinThrowable * _Nullable cause __attribute__((swift_name("cause")));
@property (readonly) NSString * _Nullable message __attribute__((swift_name("message")));
- (NSError *)asError __attribute__((swift_name("asError()")));
@end

__attribute__((swift_name("KotlinException")))
@interface StringGuardKotlinException : StringGuardKotlinThrowable
- (instancetype)init __attribute__((swift_name("init()"))) __attribute__((objc_designated_initializer));
+ (instancetype)new __attribute__((availability(swift, unavailable, message="use object initializers instead")));
- (instancetype)initWithMessage:(NSString * _Nullable)message __attribute__((swift_name("init(message:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithCause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(cause:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithMessage:(NSString * _Nullable)message cause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(message:cause:)"))) __attribute__((objc_designated_initializer));
@end

__attribute__((swift_name("KotlinRuntimeException")))
@interface StringGuardKotlinRuntimeException : StringGuardKotlinException
- (instancetype)init __attribute__((swift_name("init()"))) __attribute__((objc_designated_initializer));
+ (instancetype)new __attribute__((availability(swift, unavailable, message="use object initializers instead")));
- (instancetype)initWithMessage:(NSString * _Nullable)message __attribute__((swift_name("init(message:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithCause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(cause:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithMessage:(NSString * _Nullable)message cause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(message:cause:)"))) __attribute__((objc_designated_initializer));
@end

__attribute__((swift_name("KotlinIllegalStateException")))
@interface StringGuardKotlinIllegalStateException : StringGuardKotlinRuntimeException
- (instancetype)init __attribute__((swift_name("init()"))) __attribute__((objc_designated_initializer));
+ (instancetype)new __attribute__((availability(swift, unavailable, message="use object initializers instead")));
- (instancetype)initWithMessage:(NSString * _Nullable)message __attribute__((swift_name("init(message:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithCause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(cause:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithMessage:(NSString * _Nullable)message cause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(message:cause:)"))) __attribute__((objc_designated_initializer));
@end


/**
 * @note annotations
 *   kotlin.SinceKotlin(version="1.4")
*/
__attribute__((swift_name("KotlinCancellationException")))
@interface StringGuardKotlinCancellationException : StringGuardKotlinIllegalStateException
- (instancetype)init __attribute__((swift_name("init()"))) __attribute__((objc_designated_initializer));
+ (instancetype)new __attribute__((availability(swift, unavailable, message="use object initializers instead")));
- (instancetype)initWithMessage:(NSString * _Nullable)message __attribute__((swift_name("init(message:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithCause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(cause:)"))) __attribute__((objc_designated_initializer));
- (instancetype)initWithMessage:(NSString * _Nullable)message cause:(StringGuardKotlinThrowable * _Nullable)cause __attribute__((swift_name("init(message:cause:)"))) __attribute__((objc_designated_initializer));
@end

__attribute__((swift_name("KotlinIterator")))
@protocol StringGuardKotlinIterator
@required
- (BOOL)hasNext __attribute__((swift_name("hasNext()")));
- (id _Nullable)next __attribute__((swift_name("next()")));
@end

__attribute__((swift_name("KotlinByteIterator")))
@interface StringGuardKotlinByteIterator : StringGuardBase <StringGuardKotlinIterator>
- (instancetype)init __attribute__((swift_name("init()"))) __attribute__((objc_designated_initializer));
+ (instancetype)new __attribute__((availability(swift, unavailable, message="use object initializers instead")));
- (StringGuardByte *)next __attribute__((swift_name("next()")));
- (int8_t)nextByte __attribute__((swift_name("nextByte()")));
@end

#pragma pop_macro("_Nullable_result")
#pragma clang diagnostic pop
NS_ASSUME_NONNULL_END
