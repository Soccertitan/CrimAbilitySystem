// Copyright Soccertitan

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

	/**
	 * Processes all inputs that were pressed and released this frame. Activating abilities, sending InputPressed events,
	 * and InputReleased events.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/**
	 * Adds an AbilityInputItem to the container, or updates an existing one.
	 * @param Item The Item to add to the container.
	 * @param bReplaceAbilities If true, the AbilitiesToActivate is overwritten. If false, the abilities are appended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void AddAbilityInputItem(const FAbilityInputItem& Item, bool bReplaceAbilities = true);

	/**
	 * Removes the ability from all InputAbilityItems that don't exist in the InputTags. And Adds the ability to the
	 * InputAbilityItem with the InputTags.
	 * @param InputTags The InputTags to add the ability to. And by exclusion removes the ability from all AbilityInputItems
	 * that do not have the InputTag.
	 * @param Ability The ability to update.
	 * @param bReplaceAbilities If true, the AbilitiesToActivate is overwritten. If false, the abilities are appended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void UpdateAbilityInputItem(const FGameplayTagContainer& InputTags, TSoftClassPtr<UCrimGameplayAbility> Ability, bool bReplaceAbilities = true);

	/**
	 * Removes each ability from the InputAbilityItem in the item.
	 * @param Item The AbilityInputItem that has the abilities you want to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAbilityInputItem(const FAbilityInputItem& Item);
	/**
	 * Removes the entire InputAbilityItem from the container.
	 * @param InputTag The AbilityInputItem to remove entirely.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void RemoveAbilityInputItemByTag(const FGameplayTag& InputTag);

	/**
	 * Empties out the container of all Inputs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ResetAbilityInputContainer();

	/**
	 * Resets the AbilityInputContainer to default settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Crim Ability System|Input")
	void ResetAbilityInputContainerToDefaults();

	/**
	 * @param InputTag The AbilityInputItem to search for.
	 * @return A copy of the CrimAbilityInputItem.
	 */
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	FAbilityInputItem GetInputAbilityItem(const FGameplayTag& InputTag) const;
	
	// Returns true if the ability exists in the InputTag.
	UFUNCTION(BlueprintPure, Category = "Crim Ability System|Input")
	bool IsAbilityMappedToInput(const FGameplayTag& InputTag, TSoftClassPtr<UCrimGameplayAbility> Ability) const;

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

	/**
	 * On BeginPlay sets the default abilities to be mapped to the AbilityInputContainer.
	 */
	UPROPERTY(EditAnywhere, Category = "Input")
	FAbilityInputContainer DefaultAbilityInputContainer;
	// If true, resets the AbilityInputContainer before applying the DefaultAbilityInputContainer.
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

	/** Clears all inputs from the queue from being processed. */
	void ClearAbilityInput();
};
