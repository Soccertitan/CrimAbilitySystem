// Copyright Soccertitan 2026


#include "CrimGameplayAbilityTargetData.h"


bool FCrimGameplayAbilityTargetData::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	SafeNetSerializeTArray_WithNetSerialize<31>(Ar, CustomDataFragments, Map);
	
	bOutSuccess = true;
	return true;
}
