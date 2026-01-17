// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AbilityInputTypes.generated.h"

class UGameplayAbility;
class UAbilityInputManagerComponent;
struct FAbilityInputContainer;

/**
 * Maps an InputTag to an array of abilities.
 */
USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	FAbilityInputItem(){}
	FAbilityInputItem(const FGameplayTag& InInputTag, TSoftClassPtr<UGameplayAbility> InGameplayAbility);

	// The InputTag to activate the abilities.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Input"))
	FGameplayTag InputTag;

	// The GameplayAbility class to activate.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UGameplayAbility> GameplayAbilityClass;
	
	void PostReplicatedAdd(const FAbilityInputContainer& InArraySerializer);
	void PostReplicatedChange(const FAbilityInputContainer& InArraySerializer);
	void PreReplicatedRemove(const FAbilityInputContainer& InArraySerializer);

	bool IsValid() const;

	FORCEINLINE bool operator ==(FGameplayTag const& Other) const
	{
		return InputTag == Other;
	}

	FORCEINLINE bool operator !=(FGameplayTag const& Other) const
	{
		return InputTag != Other;
	}

	FORCEINLINE bool operator==(FAbilityInputItem const& Other) const
	{
		return InputTag == Other.InputTag;
	}

	FORCEINLINE bool operator!=(FAbilityInputItem const& Other) const
	{
		return InputTag != Other.InputTag;
	}
};

/**
 * A FastArray holding a collection of AbilityInputItems.
 */
USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	/**
	 * Adds a new AbilityInputItem to the container, or updates an existing one.
	 * @param Item The Item to add to the container.
	 */
	void AddAbilityInputItem(const FAbilityInputItem& Item);

	/**
	 * Removes the AbilityInputItem with the matching InputTag from the container.
	 * @param InputTag The tag to search for and remove.
	 */
	void RemoveAbilityInputItem(const FGameplayTag& InputTag);

	/** Gets a const reference of all AbilityInputs in the container. */
	const TArray<FAbilityInputItem>& GetItems() const;

	/** Empties out the container of all Inputs. */
	void Reset();

	/**
	 * @param InputTag The AbilityInputItem to search for.
	 * @return A copy of the AbilityInputItem.
	 */
	FAbilityInputItem FindInputAbilityItem(const FGameplayTag& InputTag) const;

	void RegisterWithOwner(UAbilityInputManagerComponent* Owner);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FAbilityInputItem, FAbilityInputContainer>(Items, DeltaParams, *this);
	}

private:
	// Maps InputTags to abilities.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(TitleProperty = "InputTag", AllowPrivateAccess = "true"))
	TArray<FAbilityInputItem> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UAbilityInputManagerComponent> Owner;

	friend struct FAbilityInputItem;
};

template<>
struct TStructOpsTypeTraits<FAbilityInputContainer> : public TStructOpsTypeTraitsBase2<FAbilityInputContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};
