// Copyright Soccertitan 2025


#include "Ability/AsyncTask/AbilityAsync_GameplayAbilityCooldown.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"


UAbilityAsync_GameplayAbilityCooldown* UAbilityAsync_GameplayAbilityCooldown::WaitAbilityCooldown(AActor* TargetActor, FGameplayTag WithAbilityTag, FGameplayTag WithoutAbilityTag, bool OnlyTriggerOnce)
{
	UAbilityAsync_GameplayAbilityCooldown* MyObj = NewObject<UAbilityAsync_GameplayAbilityCooldown>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->CachedWithAbilityTag = WithAbilityTag;
	MyObj->CachedCooldownTag = FGameplayTag::EmptyTag;
	MyObj->CachedWithoutAbilityTag = WithoutAbilityTag;
	MyObj->CachedAbilityClass = nullptr;
	MyObj->bCachedOnlyTriggerOnce = OnlyTriggerOnce;
	return MyObj;
}

UAbilityAsync_GameplayAbilityCooldown* UAbilityAsync_GameplayAbilityCooldown::WaitAbilityCooldownClass(AActor* TargetActor, TSubclassOf<UGameplayAbility> AbilityClass, bool OnlyTriggerOnce)
{
	UAbilityAsync_GameplayAbilityCooldown* MyObj = NewObject<UAbilityAsync_GameplayAbilityCooldown>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->CachedAbilityClass = AbilityClass;
	MyObj->CachedCooldownTag = FGameplayTag::EmptyTag;
	MyObj->bCachedOnlyTriggerOnce = OnlyTriggerOnce;
	return MyObj;
}

UAbilityAsync_GameplayAbilityCooldown* UAbilityAsync_GameplayAbilityCooldown::WaitAbilityCooldownSpecificTag(AActor* TargetActor, FGameplayTag CooldownTag, bool OnlyTriggerOnce)
{
	UAbilityAsync_GameplayAbilityCooldown* MyObj = NewObject<UAbilityAsync_GameplayAbilityCooldown>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->CachedCooldownTag = CooldownTag;
	MyObj->CachedAbilityClass = nullptr;
	MyObj->bCachedOnlyTriggerOnce = OnlyTriggerOnce;
	return MyObj;
}

void UAbilityAsync_GameplayAbilityCooldown::HandleTagChanged(FGameplayTag GameplayTag, int32 Count)
{
	//Cooldown applied
	if (Count > 0)
	{
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(GameplayTag.GetSingleTagContainer());
		TArray<float> TimesRemaining = GetAbilitySystemComponent()->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		float TimeRemaining = 0.f;
		if (TimesRemaining.Num() > 0)
		{
			TimeRemaining = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}
		}
		CooldownApplied.Broadcast(GameplayTag, TimeRemaining);
	}
	else
	{
		CooldownRemoved.Broadcast(GameplayTag, 0.f);
	}

	if (bCachedOnlyTriggerOnce)
	{
		EndAction();
	}
}

void UAbilityAsync_GameplayAbilityCooldown::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		CooldownTagsToListenFor.Reset();

		//Use supplied ability class.
		if (CachedAbilityClass != nullptr)
		{
			UGameplayAbility* GA_CDO = CachedAbilityClass->GetDefaultObject<UGameplayAbility>();
			if (GA_CDO)
			{
				const FGameplayTagContainer* CDTags = GA_CDO->GetCooldownTags();
				if (CDTags)
				{
					CooldownTagsToListenFor.AppendTags(*CDTags);
				}
			}
		}
		else if (CachedCooldownTag.IsValid())
		{
			CooldownTagsToListenFor.AddTagFast(CachedCooldownTag);
		}
		else //Search all activatable abilities.
		{
			//Make a copy, cause of ability scope locking issues.
			TArray<FGameplayAbilitySpec> Specs = ASC->GetActivatableAbilities();
			for (const FGameplayAbilitySpec& Spec : Specs)
			{
				if (Spec.Ability && Spec.Ability->GetAssetTags().HasTag(CachedWithAbilityTag) && !Spec.Ability->GetAssetTags().HasTag(CachedWithoutAbilityTag))
				{
					const FGameplayTagContainer* CDTags = Spec.Ability->GetCooldownTags();
					if (CDTags)
					{
						CooldownTagsToListenFor.AppendTags(*CDTags);
					}
				}
			}
		}

		if (CooldownTagsToListenFor.Num() > 0)
		{
			for (const FGameplayTag& Tag : CooldownTagsToListenFor)
			{
				HandleTagChanged(Tag, ASC->GetGameplayTagCount(Tag));
				BoundASCHandles.Add(Tag, ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAbilityAsync_GameplayAbilityCooldown::HandleTagChanged));
			}
		}
		else
		{
			EndAction();
		}
	}
	else
	{
		EndAction();
	}
}

void UAbilityAsync_GameplayAbilityCooldown::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (auto& Elem : BoundASCHandles)
		{
			ASC->RegisterGameplayTagEvent(Elem.Key, EGameplayTagEventType::NewOrRemoved).Remove(Elem.Value);
		}
	}
	Super::EndAction();
}