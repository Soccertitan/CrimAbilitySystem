// Copyright Soccertitan 2025


#include "Attribute/CrimAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "CrimAbilitySystemComponent.h"

UCrimAttributeSet::UCrimAttributeSet()
{
}

UWorld* UCrimAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UCrimAbilitySystemComponent* UCrimAttributeSet::GetCrimAbilitySystemComponent() const
{
	return Cast<UCrimAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

void UCrimAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (Data.Target.AbilityActorInfo.IsValid() &&
		Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
			Data.Target.AbilityActorInfo->AvatarActor.Get());
	}
}
