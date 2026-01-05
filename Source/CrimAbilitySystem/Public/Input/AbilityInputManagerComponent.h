// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "AbilityInputManagerComponent.generated.h"

class UCrimAbilitySystemComponent;
DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilityInputManagerAbilityInputItemSignature, UAbilityInputManagerComponent* /*this AIMC*/, const FAbilityInputItem& /* AbilityMapItem */);

/**
 * Manages inputs for abilities and activate abilities for inputs.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMABILITYSYSTEM_API UAbilityInputManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityInputManagerComponent();

	void InitializeAbilitySystemComponent(UCrimAbilitySystemComponent* InAbilitySystemComponent);

	/** Called when the first time an Input is added to the container. */
	FAbilityInputManagerAbilityInputItemSignature OnAbilityInputAddedDelegate;
	/** Called when the abilities list has been changed in the container. */
	FAbilityInputManagerAbilityInputItemSignature OnAbilityInputChangedDelegate;
	/** Called when the Input has been completely removed from the container. */
	FAbilityInputManagerAbilityInputItemSignature OnAbilityInputRemovedDelegate;

	/**
	 * Adds all abilities to a queue to be activated via ProcessAbilityInput
	 * @param InputTag The tag to find in the container.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputTagPressed(const FGameplayTag& InputTag);

	/**
	 * Adds all abilities to a queue to run AbilitySpecInputReleased against.
	 * @param InputTag The tag to find in the container.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void InputTagReleased(const FGameplayTag& InputTag);

	/** Clears all inputs from the queue from being processed. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ReleaseAbilityInput();

	/**
	 * Processes all inputs that were pressed and released this frame. Activating abilities, sending InputPressed events,
	 * and InputReleased events.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/**
	 * Adds an AbilityInputItem to the container, or updates an existing one.
	 * @param Item The Item to add to the container.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void AddAbilityInputItem(UPARAM(ref) const FAbilityInputItem& Item);

	/**
	 * Adds an array of AbilityInputItem to the container, or updates an existing one.
	 * @param Items The Items to add to the container.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void AddAbilityInputItems(UPARAM(ref) const TArray<FAbilityInputItem>& Items);

	/**
	 * Removes the AbilityInputItem from the container with the matching InputTag.
	 * @param InputTag The InputTag to search for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAbilityInputItem(UPARAM(ref) const FGameplayTag& InputTag);

	/**
	 * Removes each AbilityInputItem from the container with matching InputTags.
	 * @param InputTags The InputTags to search for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAbilityInputItems(UPARAM(ref) const TArray<FGameplayTag>& InputTags);

	/** Removes all AbilityInputItems with the matching InputTags from the AbilityInputItem. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAbilityInputItemsByAbilityInputItem(UPARAM(ref) const TArray<FAbilityInputItem>& Items);

	/** Empties out the container of all Inputs. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ResetAbilityInputContainer();

	/** Resets the AbilityInputContainer to default settings. */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ResetAbilityInputContainerToDefaults();

	/** Returns a copy of the AbilityInputItems */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Crim Ability System|Input")
	TArray<FAbilityInputItem> GetAbilityInputItems() const;
	
	/**
	 * @param InputTag The AbilityInputItem to search for.
	 * @return A copy of the CrimAbilityInputItem.
	 */
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	FAbilityInputItem GetAbilityInputItem(const FGameplayTag& InputTag) const;

	virtual void OnAbilityInputAdded(const FAbilityInputItem& Item);
	virtual void OnAbilityInputChanged(const FAbilityInputItem& Item);
	virtual void OnAbilityInputRemoved(const FAbilityInputItem& Item);

	// ----------------------------------------------------------------------------------------------------------------
	//  Component overrides
	// ----------------------------------------------------------------------------------------------------------------
	virtual void OnRegister() override;
	virtual void PreNetReceive() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	bool HasAuthority() const;
	
protected:
	/** Cached value of rather this is a simulated actor */
	UPROPERTY()
	bool bCachedIsNetSimulated;

	/** Caches the flags that indicate whether this component has network authority. */
	void CacheIsNetSimulated();
	
private:
	// Stores Inputs that are mapped to abilities.
	UPROPERTY(Replicated)
	FAbilityInputContainer AbilityInputContainer;

	/** The default abilities to be mapped to the AbilityInputContainer on BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Input")
	FAbilityInputContainer DefaultAbilityInputContainer;
	/** If true, will clear out the Container first before applying the DefaultAbilityInputContainer on BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bStartupOverrideAbilityInputs = false;

	UPROPERTY()
	TObjectPtr<UCrimAbilitySystemComponent> AbilitySystemComponent;

	// Handles to abilities that had their input pressed this frame mapped to an InputTag.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	// Handles to abilities that had their input released this frame mapped to an InputTag.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	// Handles to abilities that have their input held mapped to an InputTag.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	UFUNCTION(Server, Reliable)
	void Server_AddAbilityInputItem(const FAbilityInputItem& Item);

	UFUNCTION(Server, Reliable)
	void Server_AddAbilityInputItems(const TArray<FAbilityInputItem>& Items);

	UFUNCTION(Server, Reliable)
	void Server_RemoveAbilityInputItem(const FGameplayTag& InputTag);

	UFUNCTION(Server, Reliable)
	void Server_RemoveAbilityInputItems(const TArray<FGameplayTag>& InputTags);

	UFUNCTION(Server, Reliable)
	void Server_RemoveAbilityInputItemsByAbilityInputItem(const TArray<FAbilityInputItem>& Items);

	UFUNCTION(Server, Reliable)
	void Server_ResetAbilityInputContainer();

	UFUNCTION(Server, Reliable)
	void Server_ResetAbilityInputContainerToDefaults();
};
