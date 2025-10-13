// Copyright Soccertitan


#include "Input/AbilityInputManagerComponent.h"

#include "AbilityGameplayTags.h"
#include "CrimAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Net/UnrealNetwork.h"


UAbilityInputManagerComponent::UAbilityInputManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bCachedIsNetSimulated = false;
}

void UAbilityInputManagerComponent::InitializeAbilitySystemComponent(UCrimAbilitySystemComponent* InAbilitySystemComponent)
{
	AbilitySystemComponent = InAbilitySystemComponent;
}

void UAbilityInputManagerComponent::OnAbilityInputAdded(const FAbilityInputItem& Item)
{
	OnAbilityInputAddedDelegate.Broadcast(this, Item);
}

void UAbilityInputManagerComponent::OnAbilityInputChanged(const FAbilityInputItem& Item)
{
	OnAbilityInputChangedDelegate.Broadcast(this, Item);
}

void UAbilityInputManagerComponent::OnAbilityInputRemoved(const FAbilityInputItem& Item)
{
	OnAbilityInputRemovedDelegate.Broadcast(this, Item);
}

void UAbilityInputManagerComponent::InputTagPressed(const FGameplayTag& InputTag)
{
	if (AbilitySystemComponent && InputTag.IsValid())
	{
		FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
		for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilitySpec.Ability)
			{
				if (AbilityInputContainer.IsAbilityMappedToInput(InputTag, AbilitySpec.Ability->GetClass()))
				{
					InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
					InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
				}
			}
		}
	}
}

void UAbilityInputManagerComponent::InputTagReleased(const FGameplayTag& InputTag)
{
	if (AbilitySystemComponent && InputTag.IsValid())
	{
		FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
		for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilitySpec.Ability)
			{
				if (AbilityInputContainer.IsAbilityMappedToInput(InputTag, AbilitySpec.Ability->GetClass()))
				{
					InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
					InputHeldSpecHandles.Remove(AbilitySpec.Handle);
				}
			}
		}
	}
}

void UAbilityInputManagerComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (!AbilitySystemComponent ||
		AbilitySystemComponent->HasMatchingGameplayTag(FAbilityGameplayTags::Get().Ability_InputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec->Ability);
				if (CrimAbilityCDO && CrimAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.Add(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySystemComponent->AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec->Ability);

					if (CrimAbilityCDO && CrimAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.Add(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send an input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		AbilitySystemComponent->TryActivateAbility(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySystemComponent->AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	//
	// Clear the cached ability handles.
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UAbilityInputManagerComponent::AddAbilityInputItem(const FAbilityInputItem& Item, bool bReplaceAbilities)
{
	if (HasAuthority())
	{
		AbilityInputContainer.AddAbilityInputItem(Item, bReplaceAbilities);
	}
}

void UAbilityInputManagerComponent::UpdateAbilityInputItem(const FGameplayTagContainer& InputTags,
	TSoftClassPtr<UCrimGameplayAbility> Ability, bool bReplaceAbilities)
{
	if (HasAuthority())
	{
		AbilityInputContainer.UpdateAbilityInputItem(InputTags, Ability, bReplaceAbilities);
	}
}

void UAbilityInputManagerComponent::RemoveAbilityInputItem(const FAbilityInputItem& Item)
{
	if (HasAuthority())
	{
		AbilityInputContainer.RemoveAbilityInputItem(Item);
	}
}

void UAbilityInputManagerComponent::RemoveAbilityInputItemByTag(const FGameplayTag& InputTag)
{
	if (HasAuthority())
	{
		AbilityInputContainer.RemoveAbilityInputItem(InputTag);
	}
}

void UAbilityInputManagerComponent::ResetAbilityInputContainer()
{
	if (HasAuthority())
	{
		AbilityInputContainer.Reset();
	}
}

void UAbilityInputManagerComponent::ResetAbilityInputContainerToDefaults()
{
	if (HasAuthority())
	{
		AbilityInputContainer.Reset();
		for (const FAbilityInputItem& InputItem : DefaultAbilityInputContainer.Items)
		{
			AbilityInputContainer.AddAbilityInputItem(InputItem, false);
		}
	}
}

FAbilityInputItem UAbilityInputManagerComponent::GetInputAbilityItem(const FGameplayTag& InputTag) const
{
	return AbilityInputContainer.GetInputAbilityItem(InputTag);
}

bool UAbilityInputManagerComponent::IsAbilityMappedToInput(const FGameplayTag& InputTag, TSoftClassPtr<UCrimGameplayAbility> Ability) const
{
	return AbilityInputContainer.IsAbilityMappedToInput(InputTag, Ability);
}

void UAbilityInputManagerComponent::OnRegister()
{
	Super::OnRegister();
	CacheIsNetSimulated();
	AbilityInputContainer.RegisterWithOwner(this);
}

void UAbilityInputManagerComponent::PreNetReceive()
{
	// Update the cached IsNetSimulated value here if this component is still considered authority.
	// Even though the value is also cached in OnRegister and BeginPlay, clients may
	// receive properties before OnBeginPlay, so this ensures the role is correct
	// for that case.
	if (!bCachedIsNetSimulated)
	{
		CacheIsNetSimulated();
	}
}

void UAbilityInputManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AbilityInputContainer, Params);
}

void UAbilityInputManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheIsNetSimulated();

	if (HasAuthority())
	{
		if (bStartupOverrideAbilityInputs)
		{
			AbilityInputContainer.Reset();
		}
		for (const FAbilityInputItem& InputItem : DefaultAbilityInputContainer.Items)
		{
			AbilityInputContainer.AddAbilityInputItem(InputItem, false);
		}
	}
}

bool UAbilityInputManagerComponent::HasAuthority() const
{
	return !bCachedIsNetSimulated;
}

void UAbilityInputManagerComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UAbilityInputManagerComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	// Force the release of all abilities where they are waiting for input released events.
	if (AbilitySystemComponent)
	{
		for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilitySpec.Ability)
			{
				AbilitySpec.InputPressed = false;

				if (AbilitySpec.IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySystemComponent->AbilitySpecInputReleased(AbilitySpec);
				}
			}
		}
	}
}

