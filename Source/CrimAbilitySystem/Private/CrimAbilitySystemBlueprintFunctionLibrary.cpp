// Copyright Soccertitan 2025


#include "CrimAbilitySystemBlueprintFunctionLibrary.h"

#include "AbilitySystemGlobals.h"
#include "CrimAbilitySystemComponent.h"
#include "CrimGameplayEffectContext.h"
#include "Input/AbilityInputManagerComponent.h"
#include "Input/AbilityInputManagerInterface.h"
#include "StructUtils/InstancedStruct.h"

UCrimAbilitySystemComponent* UCrimAbilitySystemBlueprintFunctionLibrary::GetAbilitySystemComponent(AActor* Actor, bool LookForComponent)
{
	return Cast<UCrimAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}

UAbilityInputManagerComponent* UCrimAbilitySystemBlueprintFunctionLibrary::GetAbilityInputManagerComponent(AActor* Actor, bool LookForComponent)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}
	
	if (Actor->Implements<UAbilityInputManagerInterface>())
	{
		return IAbilityInputManagerInterface::Execute_GetAbilityInputManagerComponent(Actor);
	}

	if (LookForComponent)
	{
		// Fall back to a component search to better support BP-only actors
		return Actor->FindComponentByClass<UAbilityInputManagerComponent>();
	}

	return nullptr;
}

float UCrimAbilitySystemBlueprintFunctionLibrary::EvaluateAttributeValueWithTagsUpToChannel(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, EGameplayModEvaluationChannel Channel, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, bool& bSuccess)
{
	float RetVal = 0.f;
	if (!AbilitySystem || !AbilitySystem->HasAttributeSetForAttribute(Attribute))
	{
		bSuccess = false;
		return RetVal;
	}

	FGameplayEffectAttributeCaptureDefinition Capture(Attribute, EGameplayEffectAttributeCaptureSource::Source, true);

	FGameplayEffectAttributeCaptureSpec CaptureSpec(Capture);
	AbilitySystem->CaptureAttributeForGameplayEffect(CaptureSpec);

	FAggregatorEvaluateParameters EvalParams;

	EvalParams.SourceTags = &SourceTags;
	EvalParams.TargetTags = &TargetTags;

	bSuccess = CaptureSpec.AttemptCalculateAttributeMagnitudeUpToChannel(EvalParams, Channel, RetVal);

	return RetVal;
}

float UCrimAbilitySystemBlueprintFunctionLibrary::EvaluateAttributeValueWithTagsUpToChannelAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, EGameplayModEvaluationChannel Channel, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess)
{
	float RetVal = 0.f;
	if (!AbilitySystem || !AbilitySystem->HasAttributeSetForAttribute(Attribute))
	{
		bSuccess = false;
		return RetVal;
	}

	FGameplayEffectAttributeCaptureDefinition Capture(Attribute, EGameplayEffectAttributeCaptureSource::Source, true);

	FGameplayEffectAttributeCaptureSpec CaptureSpec(Capture);
	AbilitySystem->CaptureAttributeForGameplayEffect(CaptureSpec);

	FAggregatorEvaluateParameters EvalParams;

	EvalParams.SourceTags = &SourceTags;
	EvalParams.TargetTags = &TargetTags;

	FAggregator Aggregator;
	CaptureSpec.AttemptGetAttributeAggregatorSnapshot(Aggregator);
	Aggregator.SetBaseValue(BaseValue, false);
	RetVal = Aggregator.EvaluateToChannel(EvalParams, Channel);

	return RetVal;
}

FInstancedStruct UCrimAbilitySystemBlueprintFunctionLibrary::FindCustomDataFragmentFromContext(const UScriptStruct* FragmentType, const FGameplayEffectContextHandle& EffectContext)
{
	const FCrimGameplayEffectContext* Context = static_cast<const FCrimGameplayEffectContext*>(EffectContext.Get());
	if (Context)
	{
		for (const auto& Fragment : Context->GetCustomDataFragments())
		{
			if (Fragment.GetScriptStruct() == FragmentType)
			{
				return Fragment;
			}
		}
	}
	return FInstancedStruct();
}
