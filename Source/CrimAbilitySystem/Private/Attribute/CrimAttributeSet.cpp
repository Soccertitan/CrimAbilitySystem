// Copyright Soccertitan 2025


#include "Attribute/CrimAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CrimAbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

UCrimAttributeSet::UCrimAttributeSet()
{
}

void UCrimAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UCrimAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

UCrimAbilitySystemComponent* UCrimAttributeSet::GetCrimAbilitySystemComponent() const
{
	return Cast<UCrimAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

void UCrimAttributeSet::SendGameplayEvent(const FGameplayTag& EventTag, const FGameplayEffectModCallbackData& Data, const float Magnitude)
{
#if WITH_SERVER_CODE
	UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponentChecked();
	// Send a gameplay event through the Owner's and Instigator's ability system.
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	Payload.Target = AbilitySystemComponent->GetOwnerActor();
	Payload.ContextHandle = Data.EffectSpec.GetEffectContext();
	Payload.InstigatorTags = Data.EffectSpec.CapturedSourceTags.GetAggregatedTags() ? *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags() : FGameplayTagContainer();
	Payload.TargetTags = Data.EffectSpec.CapturedTargetTags.GetAggregatedTags() ? *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags() : FGameplayTagContainer();
	Payload.EventMagnitude = Magnitude;

	FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
	AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
#endif
}
