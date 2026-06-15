// Copyright Soccertitan 2025


#include "Input/AbilityInputManagerComponent.h"

#include "CrimAbilityNativeGameplayTags.h"
#include "CrimAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Input/AbilityInput.h"
#include "Input/AbilityInputSet.h"
#include "Net/UnrealNetwork.h"


UAbilityInputManagerComponent::UAbilityInputManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bCachedIsNetSimulated = false;
}

void UAbilityInputManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ActiveAbilityInputSet, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bActiveInputSetOverride, Params);
}

void UAbilityInputManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheIsNetSimulated();

	if (AbilityInputSets.IsEmpty())
	{
		ApplyStartupAbilityInputSets();
	}
}

void UAbilityInputManagerComponent::SetCrimAbilitySystem_Implementation(UCrimAbilitySystemComponent* InAbilitySystemComponent)
{
	if (AbilitySystemComponent == InAbilitySystemComponent)
	{
		return;
	}
	
	if (AbilitySystemComponent)
	{
		
		AbilitySystemComponent->OnAbilityGivenDelegate.RemoveAll(this);
		AbilitySystemComponent->OnAbilityRemovedDelegate.RemoveAll(this);
	}
		
	AbilitySystemComponent = InAbilitySystemComponent;
	
	if (AbilitySystemComponent)
	{
		FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
		for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			UpdateAbilitySpecHandleOnAbilityInputInstances(AbilitySpec.Ability, AbilitySpec.Handle);
		}
		
		AbilitySystemComponent->OnAbilityGivenDelegate.AddUObject(this, &UAbilityInputManagerComponent::OnAbilityGiven);
		AbilitySystemComponent->OnAbilityRemovedDelegate.AddUObject(this, &UAbilityInputManagerComponent::OnAbilityRemoved);
	}
}

void UAbilityInputManagerComponent::OnAbilityInputAdded(const FAbilityInputInstance& Item, const int32 AbilityInputSet)
{
	OnAbilityInputAddedDelegate.Broadcast(Item, AbilityInputSet);
}

void UAbilityInputManagerComponent::OnAbilityInputChanged(const FAbilityInputInstance& Item, const int32 AbilityInputSet)
{
	if (AbilityInputSet == 0)
	{
		InputSlotReleased(Item.InputSlot);
	}
	OnAbilityInputChangedDelegate.Broadcast(Item, AbilityInputSet);
}

void UAbilityInputManagerComponent::OnAbilityInputRemoved(const FAbilityInputInstance& Item, const int32 AbilityInputSet)
{
	if (AbilityInputSet == 0)
	{
		InputSlotReleased(Item.InputSlot);
	}
	OnAbilityInputRemovedDelegate.Broadcast(Item, AbilityInputSet);
}

void UAbilityInputManagerComponent::OnAbilityGiven(const FGameplayAbilitySpec& AbilitySpec)
{
	UpdateAbilitySpecHandleOnAbilityInputInstances(AbilitySpec.Ability, AbilitySpec.Handle);
}

void UAbilityInputManagerComponent::OnAbilityRemoved(const FGameplayAbilitySpec& AbilitySpec)
{
	UpdateAbilitySpecHandleOnAbilityInputInstances(AbilitySpec.Ability, FGameplayAbilitySpecHandle());
}

void UAbilityInputManagerComponent::RestoreActiveAbilityInputSet()
{
	for (FAbilityInputContainer& InputSet : AbilityInputSets)
	{
		if (InputSet.GetInputSet() == ActiveInputSet)
		{
			SetAbilityInputInstances(InputSet.Items, 0);
		}
	}
}

void UAbilityInputManagerComponent::InputPressed(const UAbilityInput* AbilityInput)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpecHandle Handle = FindAbilitySpecHandle(AbilityInput);
		if (Handle.IsValid())
		{
			InternalInputPressed(Handle, AbilityInput);
		}
	}
}

void UAbilityInputManagerComponent::InputSlotPressed(const FAbilityInputSlot& InputSlot, const int32 InputSet)
{
	if (AbilitySystemComponent && InputSlot.IsValid())
	{
		if (InputSet == 0)
		{
			FAbilityInputInstance* Instance = ActiveAbilityInputSet.FindInputAbilityInstance(InputSlot);
			if (Instance && Instance->AbilitySpecHandle.IsValid())
			{
				InternalInputPressed(Instance->AbilitySpecHandle, Instance->AbilityInput);
			}
		}
		else
		{
			for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
			{
				if (AbilityInputSet.GetInputSet() == InputSet)
				{
					FAbilityInputInstance* Instance = AbilityInputSet.FindInputAbilityInstance(InputSlot);
					if (Instance && Instance->AbilitySpecHandle.IsValid())
					{
						InternalInputPressed(Instance->AbilitySpecHandle, Instance->AbilityInput);
					}
				}
			}
		}
	}
}

void UAbilityInputManagerComponent::InputReleased(const UAbilityInput* AbilityInput)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpecHandle Handle = FindAbilitySpecHandle(AbilityInput);
		if (Handle.IsValid())
		{
			InternalInputReleased(Handle);
		}
	}
}

void UAbilityInputManagerComponent::InputSlotReleased(const FAbilityInputSlot& InputSlot, const int32 InputSet)
{
	if (AbilitySystemComponent && InputSlot.IsValid())
	{
		if (InputSet == 0)
		{
			FAbilityInputInstance* Instance = ActiveAbilityInputSet.FindInputAbilityInstance(InputSlot);
			if (Instance && Instance->AbilitySpecHandle.IsValid())
			{
				InternalInputReleased(Instance->AbilitySpecHandle);
			}
		}
		else
		{
			for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
			{
				if (AbilityInputSet.GetInputSet() == InputSet)
				{
					FAbilityInputInstance* Instance = AbilityInputSet.FindInputAbilityInstance(InputSlot);
					if (Instance && Instance->AbilitySpecHandle.IsValid())
					{
						InternalInputReleased(Instance->AbilitySpecHandle);
					}
				}
			}
		}
	}
}

void UAbilityInputManagerComponent::ProcessAbilityInput()
{
	if (!AbilitySystemComponent ||
		AbilitySystemComponent->HasMatchingGameplayTag(CrimAbility::NativeGameplayTag::Ability_InputBlocked))
	{
		ReleaseAbilityInput();
		return;
	}

	TArray<FAbilityInputHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset(InputPressedHandles.Num() + InputHeldHandles.Num());

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FAbilityInputHandle& AbilityInputHandle : InputHeldHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityInputHandle.Handle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec->Ability);
				if (CrimAbilityCDO && CrimAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.Add(AbilityInputHandle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FAbilityInputHandle& AbilityInputHandle : InputPressedHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityInputHandle.Handle))
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
							AbilitiesToActivate.Add(AbilityInputHandle);
						}
					}
					else
					{
						AbilitiesToActivate.Add(AbilityInputHandle);
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
	for (const FAbilityInputHandle& AbilityInputHandle : AbilitiesToActivate)
	{
		const FGameplayEventData* EventData = AbilityInputHandle.bSendGameplayEventData ? &AbilityInputHandle.EventData : nullptr;
		AbilitySystemComponent->TryActivateAbilityWithEventData(AbilityInputHandle.Handle, EventData);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedHandles)
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
	InputPressedHandles.Reset();
	InputReleasedHandles.Reset();
}

void UAbilityInputManagerComponent::SetAbilityInput(const FAbilityInputSlot& InputSlot, UAbilityInput* AbilityInput, const int32 InputSet)
{
	if (InputSlot.IsValid() && AbilityInput && InputSet >= 0)
	{
		if (InputSet == 0)
		{
			if (!HasAuthority())
			{
				ServerSetAbilityInput(InputSlot, AbilityInput, InputSet);
				return;
			}
		
			if (bActiveInputSetOverride == false)
			{
				InternalAddAbilityInputInstance(InputSlot, AbilityInput, ActiveAbilityInputSet);
			}
			return;
		}
		
		if (IsLocalClient())
		{
			bool bFoundExistingAbilityInputSet = false;
			for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
			{
				if (AbilityInputSet.GetInputSet() == InputSet)
				{
					InternalAddAbilityInputInstance(InputSlot, AbilityInput, AbilityInputSet);
					bFoundExistingAbilityInputSet = true;
					break;
				}
			}
			
			if (bFoundExistingAbilityInputSet == false)
			{
				FAbilityInputContainer& NewSet = AbilityInputSets.AddDefaulted_GetRef();
				NewSet.RegisterWithOwner(this);
				NewSet.InputSet = InputSet;
				InternalAddAbilityInputInstance(InputSlot, AbilityInput, NewSet);
				
				AbilityInputSets.Sort([](const FAbilityInputContainer& A, const FAbilityInputContainer& B)
				{
					// Sorting from small to high.
					return A.GetInputSet() < B.GetInputSet();
				});
			}
			
			if (ActiveInputSet == InputSet && bActiveInputSetOverride == false)
			{
				SetAbilityInput(InputSlot, AbilityInput, 0);
			}
		}
	}
}

void UAbilityInputManagerComponent::SetAbilityInputInstances(const TArray<FAbilityInputInstance>& AbilityInputInstances, const int32 InputSet)
{
	if (InputSet >= 0)
	{
		if (InputSet == 0)
		{
			if (!HasAuthority())
			{
				ServerSetAbilityInputInstances(AbilityInputInstances, InputSet);
				return;
			}
		
			if (bActiveInputSetOverride == false)
			{
				ActiveAbilityInputSet.Reset();
				for (const FAbilityInputInstance& Instance : AbilityInputInstances)
				{
					ActiveAbilityInputSet.AddAbilityInputInstance(Instance);
				}
			}
			return;
		}
		
		if (IsLocalClient())
		{
			bool bFoundExistingAbilityInputSet = false;
			for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
			{
				if (AbilityInputSet.GetInputSet() == InputSet)
				{
					AbilityInputSet.Reset();
					for (const FAbilityInputInstance& Instance : AbilityInputInstances)
					{
						AbilityInputSet.AddAbilityInputInstance(Instance);
					}
					bFoundExistingAbilityInputSet = true;
					break;
				}
			}
			
			if (bFoundExistingAbilityInputSet == false)
			{
				FAbilityInputContainer& NewSet = AbilityInputSets.AddDefaulted_GetRef();
				NewSet.InputSet = InputSet;
				NewSet.RegisterWithOwner(this);
				for (const FAbilityInputInstance& Instance : AbilityInputInstances)
				{
					NewSet.AddAbilityInputInstance(Instance);
				}
				
				AbilityInputSets.Sort([](const FAbilityInputContainer& A, const FAbilityInputContainer& B)
				{
					// Sorting from small to high.
					return A.GetInputSet() < B.GetInputSet();
				});
			}
			
			if (ActiveInputSet == InputSet && bActiveInputSetOverride == false)
			{
				SetAbilityInputInstances(AbilityInputInstances, 0);
			}
		}
	}
}

void UAbilityInputManagerComponent::ApplyStartupAbilityInputSets()
{
	if (IsLocalClient())
	{
		for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			AbilityInputSet.Reset();
		}
		
		for (int32 Index = 0; Index < StartupAbilityInputSets.Num(); Index++)
		{
			if (UAbilityInputSet* AbilityInputSet = StartupAbilityInputSets[Index])
			{
				for (const FAbilityInputItem& Item : AbilityInputSet->Items)
				{
					SetAbilityInput(Item.Slot, Item.AbilityInput, Index + 1);
				}
			}
		}
	}
}

void UAbilityInputManagerComponent::RemoveAbilityInput(const FAbilityInputSlot& InputSlot, const int32 InputSet)
{
	if (InputSlot.IsValid() && InputSet >= 0)
	{
		if (InputSet == 0)
		{
			if (!HasAuthority())
			{
				ServerRemoveAbilityInput(InputSlot, InputSet);
				return;
			}
		
			if (bActiveInputSetOverride == false)
			{
				ActiveAbilityInputSet.RemoveAbilityInputInstance(InputSlot);
			}
			return;
		}
		
		if (IsLocalClient())
		{
			for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
			{
				if (AbilityInputSet.GetInputSet() == InputSet)
				{
					AbilityInputSet.RemoveAbilityInputInstance(InputSlot);
					break;
				}
			}
			
			if (ActiveInputSet == InputSet && bActiveInputSetOverride == false)
			{
				RemoveAbilityInput(InputSlot, 0);
			}
		}
	}
}

void UAbilityInputManagerComponent::RemoveAllAbilityInputInstanceWithMatchingAbility(UAbilityInput* AbilityInput)
{
	if (IsLocalClient())
	{
		for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			AbilityInputSet.RemoveAbilityInputInstance(AbilityInput);
		}
	}
}

void UAbilityInputManagerComponent::SetOverrideActiveInputSet(bool bEnable, UAbilityInputSet* InputSet)
{
	if (!HasAuthority())
	{
		return;
	}
	
	bActiveInputSetOverride = bEnable;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bActiveInputSetOverride, this);
	ActiveAbilityInputSet.Reset();
	
	if (bActiveInputSetOverride)
	{
		if (InputSet)
		{
			for (const FAbilityInputItem& Item : InputSet->Items)
			{
				if (Item.AbilityInput)
				{
					InternalAddAbilityInputInstance(Item.Slot, Item.AbilityInput, ActiveAbilityInputSet);
				}
			}
		}
	}
	else
	{
		ClientRestoreActiveAbilityInputSet();
	}
}

void UAbilityInputManagerComponent::SetActiveAbilityInputSet(const int32 InputSet)
{
	if (ActiveInputSet != InputSet && InputSet > 0 && IsLocalClient())
	{
		ActiveInputSet = InputSet;
		OnInputSetChangedDelegate.Broadcast(ActiveInputSet);
		
		for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			if (AbilityInputSet.GetInputSet() == InputSet)
			{
				if (bActiveInputSetOverride == false)
				{
					SetAbilityInputInstances(AbilityInputSet.GetItems(), 0);
				}
				break;
			}
		}
	}
}

void UAbilityInputManagerComponent::SwitchToNextAbilityInputSet(const bool bIncrementInputSet)
{
	int32 NextInputSet = bIncrementInputSet ? ActiveInputSet + 1 : ActiveInputSet - 1;
	const int32 MaxInputSet = AbilityInputSets[AbilityInputSets.Num() - 1].GetInputSet();

	do
	{
		if (NextInputSet > MaxInputSet)
		{
			// Greater than max, go back to one.
			NextInputSet = 1;
		}
		else if (NextInputSet < 1)
		{
			// Less than the minimum index, loop back to max.
			NextInputSet = MaxInputSet;
		}
		else if (IsAbilityInputSetEmpty(NextInputSet))
		{
			NextInputSet += bIncrementInputSet ? 1 : -1;
		}
		else
		{
			// All conditions pass, we update the active set.
			SetActiveAbilityInputSet(NextInputSet);
		}
	}
	while (NextInputSet != ActiveInputSet);
}

bool UAbilityInputManagerComponent::IsAbilityInputSetEmpty(int32 InputSet) const
{
	for (const FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
	{
		if (AbilityInputSet.GetInputSet() == InputSet)
		{
			return AbilityInputSet.Items.Num() == 0;
		}
	}
	return true;
}

void UAbilityInputManagerComponent::ResetAbilityInputSet(int32 InputSet)
{
	if (IsLocalClient() && InputSet > 0)
	{
		for (FAbilityInputContainer& AbilityInputSet: AbilityInputSets)
		{
			if (AbilityInputSet.GetInputSet() == InputSet)
			{
				AbilityInputSet.Reset();
			}
		}
	}
}

TArray<FAbilityInputInstance> UAbilityInputManagerComponent::GetAbilityInputItems(const int32 AbilitySet) const
{
	if (AbilitySet == 0)
	{
		return ActiveAbilityInputSet.GetItems();
	}
	
	if (AbilitySet > 0)
	{
		for (const FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			if (AbilityInputSet.GetInputSet() == AbilitySet)
			{
				return AbilityInputSet.GetItems();
			}
		}
	}
	
	return TArray<FAbilityInputInstance>();
}

FAbilityInputInstance UAbilityInputManagerComponent::FindAbilityInputInstance(const FAbilityInputSlot& InputSlot, const int32 AbilitySet) const
{
	if (AbilitySet == 0)
	{
		if (FAbilityInputInstance* Instance = ActiveAbilityInputSet.FindInputAbilityInstance(InputSlot))
		{
			return *Instance;
		}
		return FAbilityInputInstance();
	}
	
	if (AbilitySet > 0)
	{
		for (const FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			if (FAbilityInputInstance* Instance = AbilityInputSet.FindInputAbilityInstance(InputSlot))
			{
				return *Instance;
			}
		}
	}
	return FAbilityInputInstance();
}

void UAbilityInputManagerComponent::OnRegister()
{
	Super::OnRegister();
	CacheIsNetSimulated();
	ActiveAbilityInputSet.RegisterWithOwner(this);
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

bool UAbilityInputManagerComponent::HasAuthority() const
{
	return !bCachedIsNetSimulated;
}

bool UAbilityInputManagerComponent::IsLocalClient() const
{
	return bLocalClient;
}

void UAbilityInputManagerComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
	
	bLocalClient = GetOwner()->HasLocalNetOwner();
}

void UAbilityInputManagerComponent::ReleaseAbilityInput()
{
	InputPressedHandles.Reset();
	InputReleasedHandles.Reset();
	InputHeldHandles.Reset();

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

FGameplayAbilitySpecHandle UAbilityInputManagerComponent::FindAbilitySpecHandle(const UAbilityInput* AbilityInput) const
{
	FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilityInput->GameplayAbility.Get() == AbilitySpec.Ability->GetClass())
		{
			return AbilitySpec.Handle;
		}
	}
	return FGameplayAbilitySpecHandle();
}

void UAbilityInputManagerComponent::InternalInputPressed(const FGameplayAbilitySpecHandle& Handle, const UAbilityInput* AbilityInput)
{
	FAbilityInputHandle InputHandle;
	InputHandle.Handle = Handle;
	if (AbilityInput->bSendGameplayEventData)
	{
		InputHandle.bSendGameplayEventData = true;
		InputHandle.EventData = AbilityInput->MakeGameplayEventData(AbilitySystemComponent);
	}
	
	InputPressedHandles.AddUnique(Handle);
	InputHeldHandles.AddUnique(Handle);
}

void UAbilityInputManagerComponent::InternalInputReleased(const FGameplayAbilitySpecHandle& Handle)
{
	InputReleasedHandles.AddUnique(Handle);
	InputHeldHandles.Remove(Handle);
}

void UAbilityInputManagerComponent::UpdateAbilitySpecHandleOnAbilityInputInstances(const UGameplayAbility* Ability, const FGameplayAbilitySpecHandle& Handle)
{
	if (HasAuthority())
	{
		for (FAbilityInputInstance& Instance : ActiveAbilityInputSet.Items)
		{
			if (Instance.AbilityInput->GameplayAbility.Get() == Ability->GetClass())
			{
				Instance.AbilitySpecHandle = Handle;
				ActiveAbilityInputSet.MarkItemDirty(Instance);
			}
		}
	}
	
	for (FAbilityInputContainer& Set : AbilityInputSets)
	{
		for (FAbilityInputInstance& Instance : Set.Items)
		{
			if (Instance.AbilityInput->GameplayAbility.Get() == Ability->GetClass())
			{
				Instance.AbilitySpecHandle = Handle;
				ActiveAbilityInputSet.MarkItemDirty(Instance);
			}
		}
	}
}

void UAbilityInputManagerComponent::ClearAllAbilitySpecHandles()
{
	if (HasAuthority())
	{
		for (FAbilityInputInstance& Instance : ActiveAbilityInputSet.Items)
		{
			Instance.AbilitySpecHandle = FGameplayAbilitySpecHandle();
			ActiveAbilityInputSet.MarkItemDirty(Instance);
		}
	}
	
	for (FAbilityInputContainer& Set : AbilityInputSets)
	{
		for (FAbilityInputInstance& Instance : Set.Items)
		{
			Instance.AbilitySpecHandle = FGameplayAbilitySpecHandle();
			ActiveAbilityInputSet.MarkItemDirty(Instance);
		}
	}
}

void UAbilityInputManagerComponent::InternalAddAbilityInputInstance(const FAbilityInputSlot& InputSlot, UAbilityInput* AbilityInput, FAbilityInputContainer& InputSet)
{
	FAbilityInputInstance NewInstance;
	NewInstance.InputSlot = InputSlot;
	NewInstance.AbilityInput = AbilityInput;
	
	if (AbilitySystemComponent)
	{
		FScopedAbilityListLock ActiveScopeLock(*AbilitySystemComponent);
		for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilityInput->GameplayAbility.Get() == AbilitySpec.Ability->GetClass())
			{
				NewInstance.AbilitySpecHandle = AbilitySpec.Handle;
				break;
			}
		}
	}
	
	InputSet.AddAbilityInputInstance(NewInstance);
}

void UAbilityInputManagerComponent::ServerSetAbilityInputInstances_Implementation(const TArray<FAbilityInputInstance>& AbilityInputInstances, const int32 InputSet)
{
	SetAbilityInputInstances(AbilityInputInstances, InputSet);
}

void UAbilityInputManagerComponent::ServerSetAbilityInput_Implementation(const FAbilityInputSlot& InputSlot, const UAbilityInput* AbilityInput, const int32 InputSet)
{
	SetAbilityInput(InputSlot, const_cast<UAbilityInput*>(AbilityInput), InputSet);
}

void UAbilityInputManagerComponent::ServerRemoveAbilityInput_Implementation(const FAbilityInputSlot& InputSlot, const int32 InputSet)
{
	RemoveAbilityInput(InputSlot, InputSet);
}

void UAbilityInputManagerComponent::ClientRestoreActiveAbilityInputSet_Implementation()
{
	RestoreActiveAbilityInputSet();
}
