// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitAbilityEnded.generated.h"

/** Wrapper around non bp exposed ended data. */
USTRUCT(BlueprintType)
struct FCrimAbilityEndedData
{
	GENERATED_BODY()

	/** Ability that ended, normally instance but could be CDO */
	UPROPERTY(BlueprintReadOnly, Category = "Ended Data")
	TObjectPtr<UGameplayAbility> AbilityThatEnded;

	/** Specific ability spec that ended */
	UPROPERTY(BlueprintReadOnly, Category = "Ended Data")
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	/** True if this was cancelled deliberately, false if it ended normally */
	UPROPERTY(BlueprintReadOnly, Category = "Ended Data")
	bool bWasCancelled;
};

/**
 * Notifies when an ability has ended.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UAbilityAsync_WaitAbilityEnded : public UAbilityAsync
{
	GENERATED_BODY()

public:
	/**
	 * Wait until the specified gameplay ability with tags is ended 
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "true"))
	static UAbilityAsync_WaitAbilityEnded* WaitForAbilityEndedWithTags(
		AActor* TargetActor,
		FGameplayTagContainer AbilityTags,
		EGameplayContainerMatchType MatchType = EGameplayContainerMatchType::Any,
		bool bOnlyTriggerOnce = false);

	/**
	 * Wait until the specified gameplay ability of class is ended
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "true"))
	static UAbilityAsync_WaitAbilityEnded* WaitForAbilityEndedOfClass(
		AActor* TargetActor,
		TSubclassOf<UGameplayAbility> AbilityClass,
		bool bOnlyTriggerOnce = false);

	/**
	 * Wait until the specified gameplay ability spec handle is ended
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "true"))
	static UAbilityAsync_WaitAbilityEnded* WaitForAbilityEndedInstance(
		AActor* TargetActor,
		FGameplayAbilitySpecHandle SpecHandle, bool bOnlyTriggerOnce = false);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncWaitAbilityEndedDelegate, FCrimAbilityEndedData, EndedData);

	UPROPERTY(BlueprintAssignable)
	FAsyncWaitAbilityEndedDelegate Ended;

protected:

	enum class EWaitEndFilterMode : uint8
	{
		ByTags,
		ByClass,
		BySpecHandle
	};

	virtual void Activate() override;
	virtual void EndAction() override;

	void OnAbilityEnded(const FAbilityEndedData& EndedData);

	UPROPERTY(Transient)
	FGameplayTagContainer FilterTags;

	UPROPERTY(Transient)
	TSubclassOf<UGameplayAbility> FilterAbilityClass;

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle FilterSpecHandle;

	EGameplayContainerMatchType FilterMatchType;

	EWaitEndFilterMode FilterMode;

	bool bOnlyTriggerOnce = false;

	FDelegateHandle MyHandle;
};
