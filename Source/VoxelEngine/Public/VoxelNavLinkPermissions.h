#pragma once

#include "CoreMinimal.h"
#include <type_traits>


UENUM(meta = (Bitflags))
enum class EVoxelNavLinkPermissions : uint8
{
	None = 0			UMETA(DisplayName = "None"),
	JumpUp = 1 << 0		UMETA(DisplayName = "Jump Up"),
	JumpDown = 1 << 1	UMETA(DisplayName = "Jump Down")
};

constexpr EVoxelNavLinkPermissions operator|(EVoxelNavLinkPermissions A, EVoxelNavLinkPermissions B)
{
	std::underlying_type_t<EVoxelNavLinkPermissions> Result = static_cast<std::underlying_type_t<EVoxelNavLinkPermissions>>(A) | static_cast<std::underlying_type_t<EVoxelNavLinkPermissions>>(B);
	return static_cast<EVoxelNavLinkPermissions>(Result);
}

constexpr EVoxelNavLinkPermissions operator&(EVoxelNavLinkPermissions A, EVoxelNavLinkPermissions B)
{
	std::underlying_type_t<EVoxelNavLinkPermissions> Result = static_cast<std::underlying_type_t<EVoxelNavLinkPermissions>>(A) & static_cast<std::underlying_type_t<EVoxelNavLinkPermissions>>(B);
	return static_cast<EVoxelNavLinkPermissions>(Result);
}

constexpr bool NavLinkPermissionsHasAllFlags(EVoxelNavLinkPermissions Mix, EVoxelNavLinkPermissions Mask)
{
	EVoxelNavLinkPermissions Combined = Mix & Mask;
	return Combined == Mask;
}

constexpr bool NavLinkPermissionsHasAnyFlag(EVoxelNavLinkPermissions Mix, EVoxelNavLinkPermissions Mask)
{
	EVoxelNavLinkPermissions Combined = Mix & Mask;
	return Combined != static_cast<EVoxelNavLinkPermissions>(0);
}
