// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Ability/CrimGameplayAbility.h"
#include "CrimAbilitySystemComponent.generated.h"


class UAbilityTagRelationshipMapping;

DECLARE_MULTICAST_DELEGATE_TwoParams(FCrimAbilitySystemAbilitySpecSignature, UCrimAbilitySystemComponent* /*this ASC*/, const FGameplayAbilitySpec& /* The Ability Spec */);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMABILITYSYSTEM_API UCrimAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCrimAbilitySystemComponent();

	FCrimAbilitySystemAbilitySpecSignature OnAbilityGivenDelegate;
	FCrimAbilitySystemAbilitySpecSignature OnAbilityRemovedDelegate;

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	typedef TFunctionRef<bool(const UCrimGameplayAbility* CrimAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);
	
	bool IsActivationGroupBlocked(EAbilityActivationGroup Group) const;
	void AddAbilityToActivationGroup(EAbilityActivationGroup Group, UCrimGameplayAbility* CrimAbility);
	void RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group, UCrimGameplayAbility* CrimAbility);
	void CancelActivationGroupAbilities(EAbilityActivationGroup Group, UCrimGameplayAbility* IgnoreCrimAbility, bool bReplicateCancelAbility);

	/** Gets the ability target data associated with the given ability handle and activation info */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/** Sets the current tag relationship mapping, if null it will clear it out */
	void SetTagRelationshipMapping(UAbilityTagRelationshipMapping* NewMapping);
	
	/** Looks at ability tags and gathers additional required and blocking tags */
	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	/**
	 * Finds the first ability with the passed in AbilityTag in the ability's AbilityTags.
	 * @param AbilityTag The ability to find.
	 * @return The ability spec or nullptr if none.
	 */
	FGameplayAbilitySpec* GetAbilitySpecWithAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * Finds all abilities with the specified AbilityTag in the Spec's Ability's AbilityTags.
	 * @param AbilityTag The tag to search for.
	 * @param OutAbilitySpecs Ability Specs.
	 */
	void GetAllAbilitySpecsWithAbilityTag(const FGameplayTag& AbilityTag, TArray<FGameplayAbilitySpec*>& OutAbilitySpecs);

	/**
	 * Find's the first ability with the Tag in the ability's DynamicSpecTags.
	 * @param DynamicTag The tag to search for.
	 */
	FGameplayAbilitySpec* GetAbilitySpecWithDynamicTag(const FGameplayTag& DynamicTag);

	/**
	 * Find's all abilities with the Tag in the ability's DynamicSpecTags.
	 * @param DynamicTag The tag to search for.
	 * @param OutAbilitySpecs 
	 */
	void GetAllAbilitySpecsWithDynamicTag(const FGameplayTag& DynamicTag, TArray<FGameplayAbilitySpec*>& OutAbilitySpecs);

	/**
	 * Find's the first ability with the matching ability class.
	 * @param AbilityClass The ability to search for.
	 */
	FGameplayAbilitySpec* GetAbilitySpecByClass(TSubclassOf<UGameplayAbility> AbilityClass);

	/**
	 * Find's all abilities with the matching ability class.
	 * @param AbilityClass The ability to search for.
	 * @param OutAbilitySpecs 
	 */
	void GetAllAbilitySpecsByClass(TSubclassOf<UGameplayAbility> AbilityClass, TArray<FGameplayAbilitySpec*>& OutAbilitySpecs);
	
	/**
	 * Adds the specified GameplayTag to the dynamic tags of the AbilitySpec in it's AbilityTags.
	 * @param AbilitySpec The ability to add the DynamicTag to.
	 * @param DynamicTag The Tag to add.
	 */
	void AddDynamicTagToAbilitySpec(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& DynamicTag);

	/**
	 * Removes the specified GameplayTag from the dynamic tags of the AbilitySpec in it's AbilityTags.
	 * @param AbilitySpec The ability to remove the specified tag from.
	 * @param DynamicTag The tag to remove.
	 */
	void RemoveDynamicTagFromAbilitySpec(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& DynamicTag);

protected:

	void TryActivateAbilitiesOnSpawn(); 

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	/** Notify client that an ability failed to activate */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

private:
	
	// Mapping of how ability tags block or cancel other abilities.
	UPROPERTY(EditAnywhere, Category = "CrimAbilitySystem")
	TObjectPtr<UAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[(uint8)EAbilityActivationGroup::MAX];
};
