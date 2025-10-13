// Copyright Soccertitan


#include "CrimAbilitySystemGlobals.h"

#include "CrimGameplayEffectContext.h"

FGameplayEffectContext* UCrimAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FCrimGameplayEffectContext();
}
