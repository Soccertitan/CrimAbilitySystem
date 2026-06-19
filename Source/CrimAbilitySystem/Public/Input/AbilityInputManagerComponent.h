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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAbilityInputManagerAbilityInputInstanceSignature, const FAbilityInputInstance&, AbilityInputInstance, const int32, InputSet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInputManagerInputSetSignature, const int32, InputSet);

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
	
	/** [Local Client Only] Called when the active input set changes. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnActiveAbilityInputSetChanged")
	FAbilityInputManagerInputSetSignature OnInputSetChangedDelegate;

	/**
	 * Adds the ability to a queue to be activated via ProcessAbilityInput
	 * @param AbilityInput The Ability that is pressed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputPressed(const UAbilityInput* AbilityInput);
	
	/**
	 * Adds all abilities to a queue to be activated via ProcessAbilityInput
	 * @param InputSlot The slot to find in the container.
	 * @param InputSet The set to use. A value of 0 is equal to using the active set.
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
	 * @param InputSet The set to use. A value of 0 is equal to using the active set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputSlotReleased(const FAbilityInputSlot& InputSlot, const int32 InputSet = 0);

	/** Clears all inputs from the queue from being processed. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ReleaseAbilityInput();

	/**
	 * Processes all inputs that were pressed and released. Activating abilities, sending InputPressed events,
	 * and InputReleased events. Must be manually called in a PlayerControllers Process Ability Input function or some 
	 * other alternative.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ProcessAbilityInput();

	/**
	 * Adds an AbilityInputItem to the container, or updates an existing one. Will not update the active set if it's currently overriden.
	 * @param Params The ability to set.
	 * @param InputSet The set to place the ability.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void SetAbilityInput(UPARAM(ref) const FAbilityInputParams& Params, const int32 InputSet = 1);
	
	/** 
	 * Updates the local InputSet with the passed in params. If bReset is true, the InputSet is reset before the new 
	 * params are added. Then it calls for the server to update the active input set if applicable
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void SetAbilityInputs(const TArray<FAbilityInputParams>& Params, const int32 InputSet = 1, const bool bReset = false);

	/** Resets the ability input sets to the startup ability input sets. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ApplyStartupAbilityInputSets();

	/**
	 * Removes the AbilityInputInstance from the set with the matching Slot. Will not update the active set if it's currently overriden.
	 * @param InputSlot The slot to remove the ability from.
	 * @param InputSet The set to remove the ability from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ClearAbilityInput(UPARAM(ref) const FAbilityInputSlot& InputSlot, const int32 InputSet = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ClearAbilityInputs(UPARAM(ref) const TArray<FAbilityInputSlot>& InputSlots, const int32 InputSet = 1);

	/** Removes all instances with the matching AbilityInput from the local AbilityInputSets. Updating the active set as required. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAllAbilityInputInstanceWithMatchingAbility(UAbilityInput* AbilityInput);
	
	/** Overrides the Active set with the passed in set. If nullptr, disables the override. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Input")
	void OverrideActiveInputSet(UAbilityInputSet* InputSet);
	
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	bool IsActiveInputSetOverriden() const { return OverrideInputSet ? true : false; }

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

	/** Empties out the InputSet of all AbilityInputInstances. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ResetAbilityInputSet(int32 InputSet);

	/** Returns a copy of the AbilityInputItems from the AbilitySet */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Crim Ability System|Input")
	TArray<FAbilityInputInstance> GetAbilityInputInstances(const int32 AbilitySet) const;
	
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
	
	/** Called in the OnRep_OverrideInputSet function when it's appropriate to send the proper ability set to the server. */
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
	
	// If valid, this InputSet overrides the active AbilityInputSet.
	UPROPERTY(ReplicatedUsing = "OnRep_OverrideInputSet")
	TObjectPtr<UAbilityInputSet> OverrideInputSet;
	UFUNCTION()
	void OnRep_OverrideInputSet();
	
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
	
	// void InternalAddAbilityInputInstance(const FAbilityInputSlot& InputSlot, UAbilityInput* AbilityInput, FAbilityInputContainer& InputSet);
	/** Updates the active ability input set from client. */
	void InternalSetActiveAbilityInputs(const TArray<FAbilityInputInstance>& Instances, const bool bReset);
	void InternalClearActiveAbilityInputs(const TArray<FAbilityInputSlot>& InputSlots);

	UFUNCTION(Server, Reliable)
	void ServerSetActiveAbilityInputs(const TArray<FAbilityInputInstance>& Instances, const bool bReset);
	UFUNCTION(Server, Reliable)
	void ServerClearActiveAbilityInputs(const TArray<FAbilityInputSlot>& InputSlots);
};
