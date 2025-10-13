// Copyright Soccertitan


#include "Input/AbilityInputTypes.h"

#include "Input/AbilityInputManagerComponent.h"

FAbilityInputItem::FAbilityInputItem(const FGameplayTag& InInputTag, TArray<TSoftClassPtr<UCrimGameplayAbility>> Abilities)
{
	InputTag = InInputTag;

	for (TSoftClassPtr<UCrimGameplayAbility> Ability : Abilities)
	{
		AbilitiesToActivate.AddUnique(Ability);
	}
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

void FAbilityInputContainer::AddAbilityInputItem(const FAbilityInputItem& Item, bool bReplaceAbilities)
{
	if (Owner && Item.InputTag.IsValid())
	{
		for (FAbilityInputItem& AbilityInputItem : Items)
		{
			if (AbilityInputItem.InputTag == Item.InputTag)
			{
				if (bReplaceAbilities)
				{
					AbilityInputItem.AbilitiesToActivate.Empty();
				}
				
				for (auto& Ability : Item.AbilitiesToActivate)
				{
					AbilityInputItem.AbilitiesToActivate.AddUnique(Ability);	
				}
				Owner->OnAbilityInputChanged(AbilityInputItem);
				MarkItemDirty(AbilityInputItem);
				return;
			}
		}
	
		FAbilityInputItem& NewItem = Items.AddDefaulted_GetRef();
		NewItem.InputTag = Item.InputTag;
		for (auto& Ability : Item.AbilitiesToActivate)
		{
			NewItem.AbilitiesToActivate.AddUnique(Ability);	
		}
		Owner->OnAbilityInputAdded(NewItem);
		MarkItemDirty(NewItem);
	}
}

void FAbilityInputContainer::UpdateAbilityInputItem(const FGameplayTagContainer& InputTags,
	TSoftClassPtr<UCrimGameplayAbility> Ability, bool bReplaceAbilities)
{
	if (Owner && InputTags.IsValid() && !Ability.IsNull())
	{
		for (const FGameplayTag& InputTag : InputTags)
		{
			AddAbilityInputItem(FAbilityInputItem(InputTag, {Ability}), bReplaceAbilities);
		}
		
		for (FAbilityInputItem& InputAbilityItem : Items)
		{
			if (!InputTags.HasTag(InputAbilityItem.InputTag))
			{
				InputAbilityItem.AbilitiesToActivate.Remove(Ability);
				Owner->OnAbilityInputChanged(InputAbilityItem);
				MarkItemDirty(InputAbilityItem);
			}
		}
	}
}

void FAbilityInputContainer::RemoveAbilityInputItem(const FAbilityInputItem& Item)
{
	if (Owner && Item.InputTag.IsValid())
	{
		for (FAbilityInputItem& InputAbilityItem : Items)
		{
			if (InputAbilityItem.InputTag == Item.InputTag)
			{
				for (auto& Ability : Item.AbilitiesToActivate)
				{
					InputAbilityItem.AbilitiesToActivate.Remove(Ability);
				}
				Owner->OnAbilityInputChanged(InputAbilityItem);
				MarkItemDirty(InputAbilityItem);
			}
		}
	}
}

void FAbilityInputContainer::RemoveAbilityInputItem(const FGameplayTag& InputTag)
{
	if (Owner && InputTag.IsValid())
	{
		for (int32 idx = Items.Num() - 1; idx >= 0; idx--)
		{
			if (Items[idx].InputTag == InputTag)
			{
				FAbilityInputItem OldItem = Items[idx];
				Items.RemoveAt(idx);
				Owner->OnAbilityInputRemoved(OldItem);
				MarkArrayDirty();
			}
		}
	}
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

FAbilityInputItem FAbilityInputContainer::GetInputAbilityItem(const FGameplayTag& InputTag) const
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

bool FAbilityInputContainer::IsAbilityMappedToInput(const FGameplayTag& InputTag, TSoftClassPtr<UCrimGameplayAbility> Ability) const
{
	for (const FAbilityInputItem& Item : Items)
	{
		if (Item.InputTag == InputTag)
		{
			return Item.AbilitiesToActivate.Contains(Ability);
		}
	}
	return false;
}

void FAbilityInputContainer::RegisterWithOwner(UAbilityInputManagerComponent* InOwner)
{
	Owner = InOwner;
}
