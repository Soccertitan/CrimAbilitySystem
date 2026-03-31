// Copyright Soccertitan 2025


#include "Attribute/CrimAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CrimAbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

UCrimAttributeSet::UCrimAttributeSet()
{
}

UCrimAbilitySystemComponent* UCrimAttributeSet::GetCrimAbilitySystemComponent() const
{
	return Cast<UCrimAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

void UCrimAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttributes(Attribute, NewValue);
}

void UCrimAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttributes(Attribute, NewValue);
}

void UCrimAttributeSet::SendGameplayEvent(const FGameplayEffectModCallbackData& Data, const FGameplayTag& EventTag)
{
#if WITH_SERVER_CODE
	UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponentChecked();
	// Send a gameplay event through the Owner's and Instigator's ability system.
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	Payload.Target = AbilitySystemComponent->GetOwnerActor();
	Payload.ContextHandle = Data.EffectSpec.GetEffectContext();
	Payload.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Payload.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
	Payload.EventMagnitude = Data.EvaluatedData.Magnitude;

	FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
	AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Data.EffectSpec.GetEffectContext().GetInstigator(), Payload.EventTag, Payload);
#endif
}
