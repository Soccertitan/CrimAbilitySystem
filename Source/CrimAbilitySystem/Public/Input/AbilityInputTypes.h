// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AbilityInputTypes.generated.h"

class UAbilityInputManagerComponent;
struct FAbilityInputContainer;
class UCrimGameplayAbility;

/**
 * Maps an InputTag to an array of abilities.
 */
USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	FAbilityInputItem(){}
	FAbilityInputItem(const FGameplayTag& InInputTag, TArray<TSoftClassPtr<UCrimGameplayAbility>> Abilities);

	// The InputTag to activate the abilities.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Input"))
	FGameplayTag InputTag;

	// The array of abilities to activate when the input is pressed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftClassPtr<UCrimGameplayAbility>> AbilitiesToActivate;
	
	void PostReplicatedAdd(const FAbilityInputContainer& InArraySerializer);
	void PostReplicatedChange(const FAbilityInputContainer& InArraySerializer);
	void PreReplicatedRemove(const FAbilityInputContainer& InArraySerializer);

	bool IsValid() const;
};

/**
 * A FastArray holding a collection of AbilityInputItems.
 */
USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	/**
	 * Adds an AbilityInputItem to the container, or updates an existing one.
	 * @param Item The Item to add to the container.
	 * @param bReplaceAbilities If true, the AbilitiesToActivate is overwritten. If false, the abilities are appended.
	 */
	void AddAbilityInputItem(const FAbilityInputItem& Item, bool bReplaceAbilities = true);

	/**
	 * Removes the ability from all InputAbilityItems that don't exist in the InputTags. And Adds the ability to the
	 * InputAbilityItem with the InputTags.
	 * @param InputTags The InputTags to add the ability to. And by exclusion removes the ability from all AbilityInputItems
	 * that do not have the InputTag.
	 * @param Ability The ability to update.
	 * @param bReplaceAbilities If true, the AbilitiesToActivate is overwritten. If false, the abilities are appended.
	 */
	void UpdateAbilityInputItem(const FGameplayTagContainer& InputTags, TSoftClassPtr<UCrimGameplayAbility> Ability, bool bReplaceAbilities = true);

	/**
	 * Removes each ability from the InputAbilityItem in the item.
	 * @param Item The AbilityInputItem that has the abilities you want to remove.
	 */
	void RemoveAbilityInputItem(const FAbilityInputItem& Item);
	/**
	 * Removes the entire InputAbilityItem from the container.
	 * @param InputTag The AbilityInputItem to remove entirely.
	 */
	void RemoveAbilityInputItem(const FGameplayTag& InputTag);

	/**
	 * Empties out the container of all Inputs.
	 */
	void Reset();

	/**
	 * @param InputTag The AbilityInputItem to search for.
	 * @return A copy of the CrimAbilityInputItem.
	 */
	FAbilityInputItem GetInputAbilityItem(const FGameplayTag& InputTag) const;
	
	// Returns true if the ability exists in the InputTag.
	bool IsAbilityMappedToInput(const FGameplayTag& InputTag, TSoftClassPtr<UCrimGameplayAbility> Ability) const;

	void RegisterWithOwner(UAbilityInputManagerComponent* Owner);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FAbilityInputItem, FAbilityInputContainer>(Items, DeltaParams, *this);
	}

	// Maps InputTags to abilities.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(TitleProperty="InputTag"))
	TArray<FAbilityInputItem> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UAbilityInputManagerComponent> Owner;
};

template<>
struct TStructOpsTypeTraits<FAbilityInputContainer> : public TStructOpsTypeTraitsBase2<FAbilityInputContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};
