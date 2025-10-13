// Copyright Soccertitan


#include "Ability/AsyncTask/AbilityCooldownEvent.h"

#include "AbilitySystemComponent.h"

UAbilityCooldownEvent* UAbilityCooldownEvent::WaitForCooldownChange(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UAbilityCooldownEvent* WaitCooldownChange = NewObject<UAbilityCooldownEvent>();
	WaitCooldownChange->AbilitySystemComponent = InAbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(InAbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// To know when a cooldown has ended (Cooldown Tag has been removed)
	InAbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange,
			&UAbilityCooldownEvent::CooldownTagChanged);

	// To know when a cooldown effect has been applied
	InAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UAbilityCooldownEvent::OnActiveEffectAdded);

	return WaitCooldownChange;
}

void UAbilityCooldownEvent::EndTask()
{
	if (!IsValid(AbilitySystemComponent)) return;
	AbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UAbilityCooldownEvent::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

void UAbilityCooldownEvent::OnActiveEffectAdded(UAbilitySystemComponent* TargetAbilitySystemComponent, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		TArray<float> TimesRemaining = AbilitySystemComponent->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			float TimeRemaining = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}

			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
