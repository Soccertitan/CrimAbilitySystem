// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputFastTypes.h"
#include "AbilityInputTypes.h"
#include "CrimAbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "AbilityInputManagerComponent.generated.h"

class UAbilityInputSet;
struct FGameplayAbilitySpec;
class UCrimAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAbilityInputManagerAbilityInputInstanceSignature, const FAbilityInputInstance&, AbilityInputInstance, const int32, AbilityInputSet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInputManagerAbilityInputSetSignature, int32, AbilitySet);

/**
 * Manages inputs for abilities and activate abilities for inputs.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMABILITYSYSTEM_API UAbilityInputManagerComponent : public UActorComponent, public ICrimAbilitySystemInterface
{
	GENERATED_BODY()
	
	friend struct FAbilityInputInstance;
	friend struct FAbilityInputContainer;

public:
	UAbilityInputManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void PreNetReceive() override;
	
	virtual void SetCrimAbilitySystem_Implementation(UCrimAbilitySystemComponent* AbilitySystemComponent) override;

	/** Called when an AbilityInput is added to an InputSet. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnAbilityInputAdded")
	FAbilityInputManagerAbilityInputInstanceSignature OnAbilityInputAddedDelegate;
	/** Called when an AbilityInputInstance changes. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnAbilityInputChanged")
	FAbilityInputManagerAbilityInputInstanceSignature OnAbilityInputChangedDelegate;
	/** Called when the Input has been completely removed from the InputSet. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnAbilityInputRemoved")
	FAbilityInputManagerAbilityInputInstanceSignature OnAbilityInputRemovedDelegate;
	
	/** [Local Client Only] */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnActiveAbilityInputSetChanged")
	FAbilityInputManagerAbilityInputSetSignature OnActiveAbilityInputSetChangedDelegate;

	/**
	 * Adds the ability to a queue to be activated via ProcessAbilityInput
	 * @param AbilityInput The Ability that is pressed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputPressed(const UAbilityInput* AbilityInput);
	
	/**
	 * Adds all abilities to a queue to be activated via ProcessAbilityInput
	 * @param InputSlot The slot to find in the container.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputSlotPressed(const FAbilityInputSlot& InputSlot, const int32 InputSet = 0);

	/**
	 * Adds the ability to a queue to run AbilitySpecInputReleased.
	 * @param AbilityInput The Ability that is pressed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputReleased(const UAbilityInput* AbilityInput);
	
	/**
	 * Adds all abilities to a queue to run AbilitySpecInputReleased against.
	 * @param InputSlot The slot to find in the container.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputSlotReleased(const FAbilityInputSlot& InputSlot, const int32 InputSet = 0);

	/** Clears all inputs from the queue from being processed. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ReleaseAbilityInput();

	/**
	 * Processes all inputs that were pressed and released. Activating abilities, sending InputPressed events,
	 * and InputReleased events.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ProcessAbilityInput();

	/**
	 * Adds an AbilityInputItem to the container, or updates an existing one. Will not update the active set if it's currently overriden.
	 * @param InputSlot The slot to put the ability.
	 * @param AbilityInput The ability to activate in the slot.
	 * @param InputSet The set to place the ability.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void SetAbilityInput(UPARAM(ref) const FAbilityInputSlot& InputSlot, UAbilityInput* AbilityInput, const int32 InputSet);
	
	/** A local player can call this to update their local AbilityInputInstance. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void SetAbilityInputInstances(const TArray<FAbilityInputInstance>& AbilityInputInstances, const int32 InputSet);

	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ApplyStartupAbilityInputSets();

	/**
	 * Removes the AbilityInputInstance from the set with the matching Slot. Will not update the active set if it's currently overriden.
	 * @param InputSlot The slot to remove the ability from.
	 * @param InputSet The set to remove the ability from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAbilityInput(UPARAM(ref) const FAbilityInputSlot& InputSlot, const int32 InputSet);

	/** Removes all instances with the matching AbilityInput from the local AbilityInputSets. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAllAbilityInputInstanceWithMatchingAbility(UAbilityInput* AbilityInput);
	
	/** Overrides the Active set with the passed in set or disables the override. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Input")
	void SetOverrideActiveInputSet(bool bEnable, UAbilityInputSet* InputSet = nullptr);
	
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	bool IsActiveInputSetOverriden() const { return bActiveInputSetOverride; }

	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void SetActiveAbilityInputSet(const int32 InputSet);

	/** 
	 * A helper function to go to the next valid In. Looping between Index 1 and the max number of Indexes. 
	 * Use SetActionSetIndex to force a switch to Index 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void SwitchToNextAbilityInputSet(const bool bIncrementInputSet = true);
	
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	int32 GetActiveInputSet() const { return ActiveInputSet; }
	
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	bool IsAbilityInputSetEmpty(int32 InputSet) const;

	/** Empties out the InputSet of all AbilityInputInstances. Will not update the ActiveSet directly. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ResetAbilityInputSet(int32 InputSet);

	/** Returns a copy of the AbilityInputItems from the AbilitySet */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Crim Ability System|Input")
	TArray<FAbilityInputInstance> GetAbilityInputItems(const int32 AbilitySet) const;
	
	/**
	 * @param InputSlot The slot to search for.
	 * @param AbilitySet The set to search within.
	 * @return A copy of the AbilityInputInstance.
	 */
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	FAbilityInputInstance FindAbilityInputInstance(const FAbilityInputSlot& InputSlot, const int32 AbilitySet) const;

	bool HasAuthority() const;
	bool IsLocalClient() const;
	
protected:
	virtual void OnRegister() override;
	
	/** Caches the flags that indicate whether this component has network authority. */
	void CacheIsNetSimulated();
	
	virtual void OnAbilityInputAdded(const FAbilityInputInstance& Item, const int32 AbilityInputSet);
	virtual void OnAbilityInputChanged(const FAbilityInputInstance& Item, const int32 AbilityInputSet);
	virtual void OnAbilityInputRemoved(const FAbilityInputInstance& Item, const int32 AbilityInputSet);
	
	void OnAbilityGiven(const FGameplayAbilitySpec& AbilitySpec);
	void OnAbilityRemoved(const FGameplayAbilitySpec& AbilitySpec);
	
	/** Called from server to tell the client to restore their active ability input set. */
	void RestoreActiveAbilityInputSet();
	
private:
	// Replicated AbilityInputs that are mapped to abilities. This is always InputSet 0.
	UPROPERTY(Replicated)
	FAbilityInputContainer ActiveAbilityInputSet;
	
	// A local only collection of AbilityInputSets. These sets will always have the InputSet greater than 0.
	UPROPERTY()
	TArray<FAbilityInputContainer> AbilityInputSets;
	
	// The currently set active set.
	UPROPERTY()
	int32 ActiveInputSet = 1;
	
	// Will be set to true by the server if the current set is overriden.
	UPROPERTY(Replicated)
	bool bActiveInputSetOverride = false;
	
	/** Cached value of rather this is a simulated actor */
	UPROPERTY()
	bool bCachedIsNetSimulated;
	
	/** Cached value if this is a locally owned connection. */
	bool bLocalClient;

	/** 
	 * The startup abilities to be mapped to the AbilityInputContainer on BeginPlay. 
	 * The first entry is treated as SlotIndex 1. Only added on the client.
	 */
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UAbilityInputSet>> StartupAbilityInputSets;

	UPROPERTY()
	TObjectPtr<UCrimAbilitySystemComponent> AbilitySystemComponent;

	// Handles to abilities that had their input pressed this frame.
	TArray<FAbilityInputHandle> InputPressedHandles;
	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedHandles;
	// Handles to abilities that have their input held.
	TArray<FAbilityInputHandle> InputHeldHandles;
	
	FGameplayAbilitySpecHandle FindAbilitySpecHandle(const UAbilityInput* AbilityInput) const;
	
	void InternalInputPressed(const FGameplayAbilitySpecHandle& Handle, const UAbilityInput* AbilityInput);
	void InternalInputReleased(const FGameplayAbilitySpecHandle& Handle);
	
	void UpdateAbilitySpecHandleOnAbilityInputInstances(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle& Handle);
	void ClearAllAbilitySpecHandles();
	
	void InternalAddAbilityInputInstance(const FAbilityInputSlot& InputSlot, UAbilityInput* AbilityInput, FAbilityInputContainer& InputSet);

	UFUNCTION(Server, Reliable)
	void ServerSetAbilityInput(const FAbilityInputSlot& InputSlot, const UAbilityInput* AbilityInput, const int32 InputSet);

	UFUNCTION(Server, Reliable)
	void ServerRemoveAbilityInput(const FAbilityInputSlot& InputSlot, const int32 InputSet);

	UFUNCTION(Client, Reliable)
	void ClientRestoreActiveAbilityInputSet();
	
	UFUNCTION(Server, Reliable)
	void ServerSetAbilityInputInstances(const TArray<FAbilityInputInstance>& AbilityInputInstances, const int32 InputSet);
};
