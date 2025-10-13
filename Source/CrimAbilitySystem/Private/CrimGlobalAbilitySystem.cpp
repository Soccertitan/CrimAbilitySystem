// Copyright Soccertitan


#include "CrimGlobalAbilitySystem.h"

#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "CrimAbilitySystemComponent.h"

void FGlobalAppliedAbilityList::AddToAbilitySystemComponent(TSubclassOf<UGameplayAbility> Ability, UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(AbilitySystemComponent))
	{
		RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
	FGameplayAbilitySpec AbilitySpec(AbilityCDO);
	const FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	Handles.Add(AbilitySystemComponent, AbilitySpecHandle);
}

void FGlobalAppliedAbilityList::RemoveFromAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(AbilitySystemComponent))
	{
		AbilitySystemComponent->ClearAbility(*SpecHandle);
		Handles.Remove(AbilitySystemComponent);
	}
}

void FGlobalAppliedAbilityList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->ClearAbility(KVP.Value);
		}
	}
	Handles.Empty();
}

void FGlobalAppliedEffectList::AddToAbilitySystemComponent(TSubclassOf<UGameplayEffect> Effect, UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(AbilitySystemComponent))
	{
		RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	const UGameplayEffect* GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffectCDO, /*Level=*/ 1, AbilitySystemComponent->MakeEffectContext());
	Handles.Add(AbilitySystemComponent, GameplayEffectHandle);
}

void FGlobalAppliedEffectList::RemoveFromAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(AbilitySystemComponent))
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(AbilitySystemComponent);
	}
}

void FGlobalAppliedEffectList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->RemoveActiveGameplayEffect(KVP.Value);
		}
	}
	Handles.Empty();
}

UCrimGlobalAbilitySystem::UCrimGlobalAbilitySystem()
{
}

void UCrimGlobalAbilitySystem::ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (!AppliedAbilities.Contains(Ability)))
	{
		FGlobalAppliedAbilityList& Entry = AppliedAbilities.Add(Ability);		
		for (UCrimAbilitySystemComponent* ASC : RegisteredAbilitySystemComponents)
		{
			Entry.AddToAbilitySystemComponent(Ability, ASC);
		}
	}
}

void UCrimGlobalAbilitySystem::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && (!AppliedEffects.Contains(Effect)))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects.Add(Effect);
		for (UCrimAbilitySystemComponent* ASC : RegisteredAbilitySystemComponents)
		{
			Entry.AddToAbilitySystemComponent(Effect, ASC);
		}
	}
}

void UCrimGlobalAbilitySystem::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && AppliedAbilities.Contains(Ability))
	{
		FGlobalAppliedAbilityList& Entry = AppliedAbilities[Ability];
		Entry.RemoveFromAll();
		AppliedAbilities.Remove(Ability);
	}
}

void UCrimGlobalAbilitySystem::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && AppliedEffects.Contains(Effect))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects[Effect];
		Entry.RemoveFromAll();
		AppliedEffects.Remove(Effect);
	}
}

void UCrimGlobalAbilitySystem::RegisterAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	check(AbilitySystemComponent);

	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.AddToAbilitySystemComponent(Entry.Key, AbilitySystemComponent);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.AddToAbilitySystemComponent(Entry.Key, AbilitySystemComponent);
	}

	RegisteredAbilitySystemComponents.AddUnique(AbilitySystemComponent);
}

void UCrimGlobalAbilitySystem::UnregisterAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	check(AbilitySystemComponent);
	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	RegisteredAbilitySystemComponents.Remove(AbilitySystemComponent);
}
