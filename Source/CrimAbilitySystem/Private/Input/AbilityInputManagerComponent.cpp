// Copyright Soccertitan 2025


#include "Input/AbilityInputManagerComponent.h"

#include "CrimAbilityNativeGameplayTags.h"
#include "CrimAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Input/AbilityInput.h"
#include "Input/AbilityInputGameplayEventData.h"
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
	Params.Condition = COND_OwnerOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OverrideInputSet, Params);
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
	InputSlotReleased(Item.InputSlot, AbilityInputSet);
	OnAbilityInputChangedDelegate.Broadcast(Item, AbilityInputSet);
}

void UAbilityInputManagerComponent::OnAbilityInputRemoved(const FAbilityInputInstance& Item, const int32 AbilityInputSet)
{
	InputSlotReleased(Item.InputSlot, AbilityInputSet);
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
	if (IsLocalClient())
	{
		for (FAbilityInputContainer& InputSet : AbilityInputSets)
		{
			if (InputSet.GetInputSet() == ActiveInputSet)
			{
				ServerSetActiveAbilityInputs(InputSet.Items, true);
			}
		}
	}
}

void UAbilityInputManagerComponent::OnRep_OverrideInputSet()
{
	if (!OverrideInputSet)
	{
		RestoreActiveAbilityInputSet();
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
				InternalInputPressed(Instance->AbilitySpecHandle, Instance->Ability);
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
						InternalInputPressed(Instance->AbilitySpecHandle, Instance->Ability);
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

void UAbilityInputManagerComponent::SetAbilityInput(const FAbilityInputParams& Params, const int32 InputSet)
{
	SetAbilityInputs({Params}, InputSet);
}

void UAbilityInputManagerComponent::SetAbilityInputs(const TArray<FAbilityInputParams>& Params, const int32 InputSet, const bool bReset)
{
	if (InputSet >= 1 && IsLocalClient())
	{
		/** Generate the array of input instances. */
		TArray<FAbilityInputInstance> AbilityInputInstances;
		AbilityInputInstances.SetNum(Params.Num());
		for (const FAbilityInputParams& Param : Params)
		{
			if (Param.Slot.IsValid())
			{
				FAbilityInputInstance Instance;
				Instance.InputSlot = Param.Slot;
				Instance.Ability = Param.Ability;
				Instance.AbilitySpecHandle = FindAbilitySpecHandle(Param.Ability);
				AbilityInputInstances.Add(Instance);
			}
		}
		
		bool bFoundExistingAbilityInputSet = false;
		for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			if (AbilityInputSet.GetInputSet() == InputSet)
			{
				if (bReset)
				{
					AbilityInputSet.Reset();
				}
				
				for (const FAbilityInputInstance& Instance : AbilityInputInstances)
				{
					AbilityInputSet.AddAbilityInputInstance(Instance);
				}
				bFoundExistingAbilityInputSet = true;
				break;
			}
		}
		
		if (!bFoundExistingAbilityInputSet)
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
		
		if (ActiveInputSet == InputSet && !OverrideInputSet)
		{
			ServerSetActiveAbilityInputs(AbilityInputInstances, bReset);
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
		AbilityInputSets.Reset(StartupAbilityInputSets.Num());
		
		for (int32 Index = 0; Index < StartupAbilityInputSets.Num(); Index++)
		{
			if (UAbilityInputSet* AbilityInputSet = StartupAbilityInputSets[Index])
			{
				SetAbilityInputs(AbilityInputSet->Items, Index + 1, true);
			}
		}
	}
}

void UAbilityInputManagerComponent::ClearAbilityInput(const FAbilityInputSlot& InputSlot, const int32 InputSet)
{
	ClearAbilityInputs({InputSlot}, InputSet);
}

void UAbilityInputManagerComponent::ClearAbilityInputs(const TArray<FAbilityInputSlot>& InputSlots, const int32 InputSet)
{
	if (InputSet >= 1 && IsLocalClient())
	{
		for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			if (AbilityInputSet.GetInputSet() == InputSet)
			{
				for (const FAbilityInputSlot& InputSlot : InputSlots)
				{
					if (InputSlot.IsValid())
					{
						AbilityInputSet.RemoveAbilityInputInstance(InputSlot);
					}
				}
				break;
			}
		}
		
		if (ActiveInputSet == InputSet && !OverrideInputSet)
		{
			ServerClearActiveAbilityInputs(InputSlots);
		}
	}
}

void UAbilityInputManagerComponent::RemoveAllAbilityInputInstanceWithMatchingAbility(UAbilityInput* AbilityInput)
{
	if (IsLocalClient())
	{
		for (FAbilityInputContainer& AbilityInputSet : AbilityInputSets)
		{
			TArray<FAbilityInputSlot> InputSlots = AbilityInputSet.RemoveAbilityInputInstance(AbilityInput);
			if (ActiveInputSet == AbilityInputSet.GetInputSet() && !OverrideInputSet)
			{
				ServerClearActiveAbilityInputs(InputSlots);
			}
		}
	}
}

void UAbilityInputManagerComponent::OverrideActiveInputSet(UAbilityInputSet* InputSet)
{
	if (!HasAuthority() && OverrideInputSet != InputSet)
	{
		return;
	}
	
	ActiveAbilityInputSet.Reset();
	OverrideInputSet = InputSet;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OverrideInputSet, this);
	
	if (OverrideInputSet)
	{
		for (const FAbilityInputParams& Item : InputSet->Items)
		{
			FAbilityInputInstance InputInstance;
			InputInstance.InputSlot = Item.Slot;
			InputInstance.Ability = Item.Ability;
			InputInstance.AbilitySpecHandle = FindAbilitySpecHandle(Item.Ability);
			ActiveAbilityInputSet.AddAbilityInputInstance(InputInstance);
		}
		return;
	}

	if (IsLocalClient())
	{
		RestoreActiveAbilityInputSet();
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
				if (!OverrideInputSet)
				{
					ServerSetActiveAbilityInputs(AbilityInputSet.GetItems(), true);
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
				if (ActiveInputSet == InputSet && !OverrideInputSet)
				{
					ServerSetActiveAbilityInputs(TArray<FAbilityInputInstance>(), true);
				}
			}
		}
	}
}

TArray<FAbilityInputInstance> UAbilityInputManagerComponent::GetAbilityInputInstances(const int32 AbilitySet) const
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
	if (AbilityInput->EventData)
	{
		InputHandle.bSendGameplayEventData = true;
		InputHandle.EventData = AbilityInput->EventData->MakeGameplayEventData(AbilitySystemComponent, AbilityInput);
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
			if (Instance.Ability && Instance.Ability->GameplayAbility.Get() == Ability->GetClass())
			{
				Instance.AbilitySpecHandle = Handle;
				ActiveAbilityInputSet.MarkItemDirty(Instance);
			}
		}
	}
	
	if (IsLocalClient())
	{
		for (FAbilityInputContainer& Set : AbilityInputSets)
		{
			for (FAbilityInputInstance& Instance : Set.Items)
			{
				if (Instance.Ability->GameplayAbility.Get() == Ability->GetClass())
				{
					Instance.AbilitySpecHandle = Handle;
					ActiveAbilityInputSet.MarkItemDirty(Instance);
				}
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
			Set.MarkItemDirty(Instance);
		}
	}
}

void UAbilityInputManagerComponent::InternalSetActiveAbilityInputs(const TArray<FAbilityInputInstance>& Instances, const bool bReset)
{
	if (!OverrideInputSet)
	{
		if (bReset)
		{
			ActiveAbilityInputSet.Reset();
		}
	
		for (const FAbilityInputInstance& Instance : Instances)
		{
			ActiveAbilityInputSet.AddAbilityInputInstance(Instance);
		}
	}
}

void UAbilityInputManagerComponent::InternalClearActiveAbilityInputs(const TArray<FAbilityInputSlot>& InputSlots)
{
	if (!OverrideInputSet)
	{
		for (const FAbilityInputSlot& InputSlot : InputSlots)
		{
			ActiveAbilityInputSet.RemoveAbilityInputInstance(InputSlot);
		}
	}
}

void UAbilityInputManagerComponent::ServerSetActiveAbilityInputs_Implementation(const TArray<FAbilityInputInstance>& Instances, const bool bReset)
{
	InternalSetActiveAbilityInputs(Instances, bReset);
}

void UAbilityInputManagerComponent::ServerClearActiveAbilityInputs_Implementation(const TArray<FAbilityInputSlot>& InputSlots)
{
	InternalClearActiveAbilityInputs(InputSlots);
}
