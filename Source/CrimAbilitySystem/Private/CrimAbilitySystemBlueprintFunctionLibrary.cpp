// Copyright Soccertitan


#include "CrimAbilitySystemBlueprintFunctionLibrary.h"

#include "AbilitySystemGlobals.h"
#include "CrimAbilitySystemComponent.h"
#include "Input/AbilityInputManagerComponent.h"
#include "Input/AbilityInputManagerInterface.h"

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
