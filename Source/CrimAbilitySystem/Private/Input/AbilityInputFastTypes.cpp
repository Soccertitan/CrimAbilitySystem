// Copyright Soccertitan 2025


#include "Input/AbilityInputFastTypes.h"

#include "Input/AbilityInputManagerComponent.h"


void FAbilityInputInstance::PostReplicatedAdd(const FAbilityInputContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAbilityInputAdded(*this, InArraySerializer.InputSet);
	}
}

void FAbilityInputInstance::PostReplicatedChange(const FAbilityInputContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAbilityInputChanged(*this, InArraySerializer.InputSet);
	}
}

void FAbilityInputInstance::PreReplicatedRemove(const FAbilityInputContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAbilityInputRemoved(*this, InArraySerializer.InputSet);
	}
}

void FAbilityInputContainer::AddAbilityInputInstance(const FAbilityInputInstance& Instance)
{
	if (Owner)
	{
		for (FAbilityInputInstance& Item : Items)
		{
			if (Item.InputSlot == Instance.InputSlot)
			{
				Item.Ability = Instance.Ability;
				Item.AbilitySpecHandle = Instance.AbilitySpecHandle;
				Owner->OnAbilityInputChanged(Item, InputSet);
				MarkItemDirty(Item);
				return;
			}
		}
	
		FAbilityInputInstance& NewItem = Items.AddDefaulted_GetRef();
		NewItem = Instance;
		Owner->OnAbilityInputAdded(NewItem, InputSet);
		MarkItemDirty(NewItem);
	}
}

void FAbilityInputContainer::RemoveAbilityInputInstance(const FAbilityInputSlot& InputSlot)
{
	if (Owner)
	{
		for (int32 Idx = Items.Num() - 1; Idx >= 0; Idx--)
		{
			if (Items[Idx].InputSlot == InputSlot)
			{
				FAbilityInputInstance OldItem = Items[Idx];
				Items.RemoveAt(Idx);
				Owner->OnAbilityInputRemoved(OldItem, InputSet);
				MarkArrayDirty();
			}
		}
	}
}

TArray<FAbilityInputSlot> FAbilityInputContainer::RemoveAbilityInputInstance(UAbilityInput* AbilityInput)
{
	TArray<FAbilityInputSlot> Result;
	if (Owner)
	{
		for (int32 Idx = Items.Num() - 1; Idx >= 0; Idx--)
		{
			if (Items[Idx].Ability == AbilityInput)
			{
				FAbilityInputInstance OldItem = Items[Idx];
				Items.RemoveAt(Idx);
				Owner->OnAbilityInputRemoved(OldItem, InputSet);
				Result.Add(OldItem.InputSlot);
				MarkArrayDirty();
			}
		}
	}
	return Result;
}

const TArray<FAbilityInputInstance>& FAbilityInputContainer::GetItems() const
{
	return Items;
}

void FAbilityInputContainer::Reset()
{
	if (Owner)
	{
		TArray<FAbilityInputInstance> TempEntries = Items;
		Items.Empty();
		for (FAbilityInputInstance& Entry : TempEntries)
		{
			Owner->OnAbilityInputRemoved(Entry, InputSet);
		}
		MarkArrayDirty();
	}
}

FAbilityInputInstance* FAbilityInputContainer::FindInputAbilityInstance(const FAbilityInputSlot& InputSlot) const
{
	for (const FAbilityInputInstance& Instance : Items)
	{
		if (Instance.InputSlot == InputSlot)
		{
			return const_cast<FAbilityInputInstance*>(&Instance);
		}
	}
	return nullptr;
}

void FAbilityInputContainer::RegisterWithOwner(UAbilityInputManagerComponent* InOwner)
{
	Owner = InOwner;
}
