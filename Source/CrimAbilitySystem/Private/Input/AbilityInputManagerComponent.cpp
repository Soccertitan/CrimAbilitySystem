// Copyright Soccertitan 2025


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
	InputTagReleased(Item.InputTag);
	OnAbilityInputChangedDelegate.Broadcast(this, Item);
}

void UAbilityInputManagerComponent::OnAbilityInputRemoved(const FAbilityInputItem& Item)
{
	InputTagReleased(Item.InputTag);
	OnAbilityInputRemovedDelegate.Broadcast(this, Item);
}

void UAbilityInputManagerComponent::InputTagPressed(const FGameplayTag& InputTag)
{
	if (AbilitySystemComponent && InputTag.IsValid())
	{
		const TSoftClassPtr<UGameplayAbility> AbilityClass = AbilityInputContainer.FindInputAbilityItem(InputTag).GameplayAbilityClass;
		if (!AbilityClass.IsNull())
		{
			FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
			for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
			{
				if (AbilitySpec.Ability && AbilitySpec.Ability->GetClass() == AbilityClass)
				{
					InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
					InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
					return;
				}
			}
		}
	}
}

void UAbilityInputManagerComponent::InputTagReleased(const FGameplayTag& InputTag)
{
	if (AbilitySystemComponent && InputTag.IsValid())
	{
		const TSoftClassPtr<UGameplayAbility> AbilityClass = AbilityInputContainer.FindInputAbilityItem(InputTag).GameplayAbilityClass;
		if (!AbilityClass.IsNull())
		{
			FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
			for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
			{
				if (AbilitySpec.Ability && AbilitySpec.Ability->GetClass() == AbilityClass)
				{
					InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
					InputHeldSpecHandles.Remove(AbilitySpec.Handle);
					return;
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
		ReleaseAbilityInput();
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
					// Activate abilities that are on InputTriggered or if it's not a CrimGameplayAbility as a fallback.
					if (const UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec->Ability))
					{
						if (CrimAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::OnInputTriggered)
						{
							AbilitiesToActivate.Add(AbilitySpec->Handle);
						}
					}
					else
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

void UAbilityInputManagerComponent::AddAbilityInputItem(const FAbilityInputItem& Item)
{
	if (!HasAuthority())
	{
		Server_AddAbilityInputItem(Item);
		return;
	}
	AbilityInputContainer.AddAbilityInputItem(Item);
}

void UAbilityInputManagerComponent::AddAbilityInputItems(const TArray<FAbilityInputItem>& Items)
{
	if (!HasAuthority())
	{
		Server_AddAbilityInputItems(Items);
		return;
	}
	for (const FAbilityInputItem& Item : Items)
	{
		AbilityInputContainer.AddAbilityInputItem(Item);
	}
}

void UAbilityInputManagerComponent::RemoveAbilityInputItem(const FGameplayTag& InputTag)
{
	if (!HasAuthority())
	{
		Server_RemoveAbilityInputItem(InputTag);
		return;
	}
	AbilityInputContainer.RemoveAbilityInputItem(InputTag);
}

void UAbilityInputManagerComponent::RemoveAbilityInputItems(const TArray<FGameplayTag>& InputTags)
{
	if (!HasAuthority())
	{
		Server_RemoveAbilityInputItems(InputTags);
		return;
	}
	for (const FGameplayTag& InputTag : InputTags)
	{
		AbilityInputContainer.RemoveAbilityInputItem(InputTag);
	}
}

void UAbilityInputManagerComponent::RemoveAbilityInputItemsByAbilityInputItem(const TArray<FAbilityInputItem>& Items)
{
	if (!HasAuthority())
	{
		Server_RemoveAbilityInputItemsByAbilityInputItem_Implementation(Items);
		return;
	}
	for (const FAbilityInputItem& Item : Items)
	{
		AbilityInputContainer.RemoveAbilityInputItem(Item.InputTag);
	}
}

void UAbilityInputManagerComponent::ResetAbilityInputContainer()
{
	if (!HasAuthority())
	{
		Server_ResetAbilityInputContainer();
		return;
	}
	AbilityInputContainer.Reset();
}

void UAbilityInputManagerComponent::ResetAbilityInputContainerToDefaults()
{
	if (!HasAuthority())
	{
		Server_ResetAbilityInputContainerToDefaults();
		return;
	}
	AbilityInputContainer.Reset();
	for (const FAbilityInputItem& InputItem : StartupAbilityInputContainer.GetItems())
	{
		AbilityInputContainer.AddAbilityInputItem(InputItem);
	}
}

TArray<FAbilityInputItem> UAbilityInputManagerComponent::GetAbilityInputItems() const
{
	return AbilityInputContainer.GetItems();
}

FAbilityInputItem UAbilityInputManagerComponent::GetAbilityInputItem(const FGameplayTag& InputTag) const
{
	return AbilityInputContainer.FindInputAbilityItem(InputTag);
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
		if (AbilityInputContainer.GetItems().IsEmpty())
		{
			for (const FAbilityInputItem& InputItem : StartupAbilityInputContainer.GetItems())
			{
				AbilityInputContainer.AddAbilityInputItem(InputItem);
			}
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

void UAbilityInputManagerComponent::ReleaseAbilityInput()
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

void UAbilityInputManagerComponent::Server_AddAbilityInputItem_Implementation(const FAbilityInputItem& Item)
{
	AddAbilityInputItem(Item);
}

void UAbilityInputManagerComponent::Server_AddAbilityInputItems_Implementation(const TArray<FAbilityInputItem>& Items)
{
	AddAbilityInputItems(Items);
}

void UAbilityInputManagerComponent::Server_RemoveAbilityInputItem_Implementation(const FGameplayTag& InputTag)
{
	RemoveAbilityInputItem(InputTag);
}

void UAbilityInputManagerComponent::Server_RemoveAbilityInputItems_Implementation(const TArray<FGameplayTag>& InputTags)
{
	RemoveAbilityInputItems(InputTags);
}

void UAbilityInputManagerComponent::Server_RemoveAbilityInputItemsByAbilityInputItem_Implementation(const TArray<FAbilityInputItem>& Items)
{
	RemoveAbilityInputItemsByAbilityInputItem(Items);
}

void UAbilityInputManagerComponent::Server_ResetAbilityInputContainer_Implementation()
{
	ResetAbilityInputContainer();
}

void UAbilityInputManagerComponent::Server_ResetAbilityInputContainerToDefaults_Implementation()
{
	ResetAbilityInputContainerToDefaults();
}
