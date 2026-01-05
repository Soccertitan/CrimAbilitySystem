// Copyright Soccertitan 2025


#include "Input/AbilityInputTypes.h"

#include "Input/AbilityInputManagerComponent.h"


FAbilityInputItem::FAbilityInputItem(const FGameplayTag& InInputTag, TObjectPtr<UGameplayAbilityDefinition> InAbilityDefinition)
{
	InputTag = InInputTag;
	AbilityDefinition = InAbilityDefinition;
}

void FAbilityInputItem::PostReplicatedAdd(const FAbilityInputContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAbilityInputAdded(*this);
	}
}

void FAbilityInputItem::PostReplicatedChange(const FAbilityInputContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAbilityInputChanged(*this);
	}
}

void FAbilityInputItem::PreReplicatedRemove(const FAbilityInputContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAbilityInputRemoved(*this);
	}
}

bool FAbilityInputItem::IsValid() const
{
	return InputTag.IsValid();
}

void FAbilityInputContainer::AddAbilityInputItem(const FAbilityInputItem& Item)
{
	if (Owner && Item.InputTag.IsValid())
	{
		for (FAbilityInputItem& AbilityInputItem : Items)
		{
			if (AbilityInputItem.InputTag == Item.InputTag)
			{
				AbilityInputItem.AbilityDefinition = Item.AbilityDefinition;
				Owner->OnAbilityInputChanged(AbilityInputItem);
				MarkItemDirty(AbilityInputItem);
				return;
			}
		}
	
		FAbilityInputItem& NewItem = Items.AddDefaulted_GetRef();
		NewItem = Item;
		Owner->OnAbilityInputAdded(NewItem);
		MarkItemDirty(NewItem);
	}
}

void FAbilityInputContainer::RemoveAbilityInputItem(const FGameplayTag& InputTag)
{
	if (Owner && InputTag.IsValid())
	{
		for (int32 Idx = Items.Num() - 1; Idx >= 0; Idx--)
		{
			if (Items[Idx].InputTag == InputTag)
			{
				FAbilityInputItem OldItem = Items[Idx];
				Items.RemoveAt(Idx);
				Owner->OnAbilityInputRemoved(OldItem);
				MarkArrayDirty();
			}
		}
	}
}

const TArray<FAbilityInputItem>& FAbilityInputContainer::GetItems() const
{
	return Items;
}

void FAbilityInputContainer::Reset()
{
	if (Owner)
	{
		TArray<FAbilityInputItem> TempEntries = Items;
		Items.Empty();
		for (FAbilityInputItem& Entry : TempEntries)
		{
			Owner->OnAbilityInputRemoved(Entry);
		}
		MarkArrayDirty();
	}
}

FAbilityInputItem FAbilityInputContainer::FindInputAbilityItem(const FGameplayTag& InputTag) const
{
	for (const FAbilityInputItem& InputAbilityItem : Items)
	{
		if (InputAbilityItem.InputTag == InputTag)
		{
			return InputAbilityItem;
		}
	}
	return FAbilityInputItem();
}

void FAbilityInputContainer::RegisterWithOwner(UAbilityInputManagerComponent* InOwner)
{
	Owner = InOwner;
}
