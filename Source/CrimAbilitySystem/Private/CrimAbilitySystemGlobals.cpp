// Copyright Soccertitan 2025


#include "CrimAbilitySystemGlobals.h"

#include "CrimGameplayEffectContext.h"

FGameplayEffectContext* UCrimAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FCrimGameplayEffectContext();
}
