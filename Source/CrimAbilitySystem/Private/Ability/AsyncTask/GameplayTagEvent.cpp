// Copyright Soccertitan


#include "Ability/AsyncTask/GameplayTagEvent.h"

#include "AbilitySystemComponent.h"

UGameplayTagEvent* UGameplayTagEvent::ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* InAbilitySystemComponent, FGameplayTagContainer InTags)
{
	UGameplayTagEvent* ListenForGameplayTagAddedRemoved = NewObject<UGameplayTagEvent>();
	ListenForGameplayTagAddedRemoved->AbilitySystemComponent = InAbilitySystemComponent;
	ListenForGameplayTagAddedRemoved->Tags = InTags;

	if (!IsValid(InAbilitySystemComponent) || InTags.Num() < 1)
	{
		ListenForGameplayTagAddedRemoved->EndTask();
		return nullptr;
	}

	TArray<FGameplayTag> TagArray;
	InTags.GetGameplayTagArray(TagArray);

	for (FGameplayTag Tag : TagArray)
	{
		InAbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(ListenForGameplayTagAddedRemoved, &UGameplayTagEvent::TagChanged);
	}

	return ListenForGameplayTagAddedRemoved;
}

void UGameplayTagEvent::EndTask()
{
	if (IsValid(AbilitySystemComponent))
	{
		TArray<FGameplayTag> TagArray;
		Tags.GetGameplayTagArray(TagArray);

		for (FGameplayTag Tag : TagArray)
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UGameplayTagEvent::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		OnTagAdded.Broadcast(Tag);
	}
	else
	{
		OnTagRemoved.Broadcast(Tag);
	}
}
