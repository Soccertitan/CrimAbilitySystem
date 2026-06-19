// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputSlot.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AbilityInputFastTypes.generated.h"

class UCrimGameplayAbility;
class UAbilityInput;
class UGameplayAbility;
class UAbilityInputManagerComponent;
struct FAbilityInputContainer;


/**
 * Maps an Input Slot to an array of abilities.
 */
USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputInstance : public FFastArraySerializerItem
{
	GENERATED_BODY()
	FAbilityInputInstance(){}

	/** The slot to logically store the ability to activate. */
	UPROPERTY(BlueprintReadOnly)
	FAbilityInputSlot InputSlot;

	// The ability to activate.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilityInput> Ability;
	
	void PostReplicatedAdd(const FAbilityInputContainer& InArraySerializer);
	void PostReplicatedChange(const FAbilityInputContainer& InArraySerializer);
	void PreReplicatedRemove(const FAbilityInputContainer& InArraySerializer);
	
private:
	// Cached handle of the ability to activate.
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	
	friend FAbilityInputContainer;
	friend UAbilityInputManagerComponent;
};

/**
 * A FastArray holding a collection of AbilityInputInstances.
 */
USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	/**
	 * Adds a new AbilityInputInstance to the container, or updates an existing one.
	 * @param Instance The Instance to add to the container.
	 */
	void AddAbilityInputInstance(const FAbilityInputInstance& Instance);

	/**
	 * Removes the AbilityInputInstance with the matching InputTag from the container.
	 * @param InputSlot The slot to search for and remove.
	 */
	void RemoveAbilityInputInstance(const FAbilityInputSlot& InputSlot);
	
	/** Removes all instances with matching AbilityInput. Returns the slots that were removed. */
	TArray<FAbilityInputSlot> RemoveAbilityInputInstance(UAbilityInput* AbilityInput);

	/** Gets a const reference of all AbilityInputs in the container. */
	const TArray<FAbilityInputInstance>& GetItems() const;
	
	int32 GetInputSet() const { return InputSet; }

	/** Empties out the container of all Inputs. */
	void Reset();

	/**
	 * @param InputSlot The AbilityInputInstance to search for.
	 * @return A pointer to the first AbilityInputInstance.
	 */
	FAbilityInputInstance* FindInputAbilityInstance(const FAbilityInputSlot& InputSlot) const;

	void RegisterWithOwner(UAbilityInputManagerComponent* Owner);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FAbilityInputInstance, FAbilityInputContainer>(Items, DeltaParams, *this);
	}

private:
	// Maps InputSlot to abilities.
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TArray<FAbilityInputInstance> Items;
	
	// The input set. Always 0 for the active set.
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	int32 InputSet = 0;

	UPROPERTY(NotReplicated)
	TObjectPtr<UAbilityInputManagerComponent> Owner;

	friend class UAbilityInputManagerComponent;
	friend struct FAbilityInputInstance;
};

template<>
struct TStructOpsTypeTraits<FAbilityInputContainer> : public TStructOpsTypeTraitsBase2<FAbilityInputContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};
