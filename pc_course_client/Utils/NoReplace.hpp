#pragma once

struct NoCopy
{
	NoCopy() = default;
	NoCopy(const NoCopy &) = delete;
	NoCopy(NoCopy &&) = default;
	NoCopy & operator=(const NoCopy &) = delete;
	NoCopy & operator=(NoCopy &&) = default;
};

struct NoMove
{
	NoMove() = default;
	NoMove(const NoMove &) = default;
	NoMove(NoMove &&) = delete;
	NoMove & operator=(const NoMove &) = default;
	NoMove & operator=(NoMove &&) = delete;
};

