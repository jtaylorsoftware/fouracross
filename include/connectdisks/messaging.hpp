#pragma once

#include <array>
#include <cstdint>

namespace connectdisks
{
	namespace server
	{
		enum class Response : uint8_t
		{
			error, connected, gameStart, gameEnd, takeTurn, turnResult, update
		};
		struct Message
		{
			Response response;
			std::array<uint8_t, 16> data;
		};
	}

	namespace client
	{
		enum class Response : uint8_t
		{
			error, ready, turn
		};

		struct Message
		{
			Response response;
			std::array<uint8_t, 16> data;
		};
	}
}