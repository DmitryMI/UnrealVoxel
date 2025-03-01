#pragma once

#include "CoreMinimal.h"
#include <type_traits>

namespace VoxelEngine::Navigation
{
	UENUM(meta = (Bitflags))
	enum class ENavLinkPermissions : uint8
	{
		None = 0			UMETA(DisplayName = "None"),
		JumpUp = 1 << 0		UMETA(DisplayName = "Jump Up"),
		JumpDown = 1 << 1	UMETA(DisplayName = "Jump Down")
	};

	constexpr ENavLinkPermissions operator|(ENavLinkPermissions A, ENavLinkPermissions B)
	{
		std::underlying_type_t<ENavLinkPermissions> Result = static_cast<std::underlying_type_t<ENavLinkPermissions>>(A) | static_cast<std::underlying_type_t<ENavLinkPermissions>>(B);
		return static_cast<ENavLinkPermissions>(Result);
	}

	constexpr ENavLinkPermissions operator&(ENavLinkPermissions A, ENavLinkPermissions B)
	{
		std::underlying_type_t<ENavLinkPermissions> Result = static_cast<std::underlying_type_t<ENavLinkPermissions>>(A) & static_cast<std::underlying_type_t<ENavLinkPermissions>>(B);
		return static_cast<ENavLinkPermissions>(Result);
	}

	constexpr bool HasFlags(ENavLinkPermissions Mix, ENavLinkPermissions Mask)
	{
		ENavLinkPermissions Combined = Mix & Mask;
		return Combined == Mask;
	}
}