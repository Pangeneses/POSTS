module;

#include <fstream>
#include <string>

#include <memory>

#include <iterator>
#include <map>
#include <vector>
#include <list>

#include <cmath>

module CHV4DARCHIVE:CHV4DENCLZSS;

import :CHV4DFORWARD;
import :CHV4DRESOURCE;

import :CHV4DBITSTREAM;

namespace CHV4DARCHIVE
{
	ZIP_ERROR CHV4DENCLZSS::AppendBlockToStream(
		std::shared_ptr<std::vector<unsigned char>> block,
		std::shared_ptr<CHV4DARCHIVE::CHV4DBITSTREAM> out,
		int16_t windowSz,
		DEFLATE_COMPRESSION method)

	{
		ZIP_ERROR error = CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

		Method = method;

		WindowSz = windowSz;

		Block.reset(block.get());

		Out.reset(out.get());

		CItt = Block->begin();

		if (method == CHV4DARCHIVE::DEFLATE_COMPRESSION_NO)
		{
			AppendNoCompression();

		}

		while (Block->size() >= 256)
		{
			Literal.clear();

			Literal.push_back(*Block.get()->begin());

			Literal.push_back(*std::next(Block->begin(), 1));

			error = IndexWindowSearch();

			if (error != CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED) return error;

			if (Index.size() > 0)
			{
				error = IndexedWindowSearch();

				if (error != CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED) return error;

			}

			error = PushLiteral();

			if (error != CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED) return error;

			error = SlideWindow();

			if (error != CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED) return error;

		}

		if (CItt != Block->end()) AppendNoCompression();

		return CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

	}

	ZIP_ERROR CHV4DENCLZSS::AppendNoCompression()
	{
		ZIP_ERROR error = CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

		Out->InsertBits(0, *Block.get(), 8);

		Window.insert(Window.end(), Block->begin(), Block->end());

		if (Window.size() > 32768)
		{
			Window.erase(Window.begin(), std::next(Window.begin(), (Window.size() - 32768)));

		}		

		return error;

	}

	ZIP_ERROR CHV4DENCLZSS::ResetWindow()
	{
		Window.clear();

		return CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

	}

	ZIP_ERROR CHV4DENCLZSS::SlideWindow()
	{
		Window.insert(Window.end(), Literal.begin(), Literal.end());

		if (Window.size() > 32768)
		{
			Window.erase(Window.begin(), std::next(Window.begin(), Window.size() - 32768));

		}

		CItt = std::next(CItt, Literal.size());

		return CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

	}

	ZIP_ERROR CHV4DENCLZSS::IndexWindowSearch()
	{
		Index.clear();

		std::list<unsigned char>::iterator itt;

		for (itt = Window.begin(); itt != --Window.end(); ++itt)
		{
			if (*itt == *Literal.begin() && *std::next(itt, 1) == *std::next(Literal.begin(), 1)) Index.push_back(itt);

		}

		if (*itt == *Literal.begin() && *CItt == *std::next(Literal.begin(), 1)) Index.push_back(itt);

		return CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

	}

	ZIP_ERROR CHV4DENCLZSS::IndexedWindowSearch()
	{
		std::list<std::list<unsigned char>::iterator>::iterator itt;

		std::list<unsigned char>::iterator jtt;

		std::list<unsigned char>::iterator FirstLongest;

		while (!Index.empty() || Literal.size() == 257)
		{
			Literal.push_back(*std::next(CItt, Literal.size()));

			FirstLongest = *Index.begin();

			for (itt = Index.begin(); itt != Index.end(); ++itt)
			{
				for (jtt = Literal.begin(); jtt != Literal.end(); ++jtt)
				{
					if (std::distance(*itt, Window.end()) >= static_cast<int64_t>(Literal.size()))
					{
						if (*jtt != *std::next(*itt, std::distance(Literal.begin(), jtt))) break;

					}
					else
					{
						break;

					}

				}

				if (jtt != Literal.end()) Index.erase(itt);

			}

		}

		Index.clear();

		Index.push_back(FirstLongest);
		
		return CHV4DARCHIVE::ZIP_ERROR_SUCCEEDED;

	}

	ZIP_ERROR CHV4DENCLZSS::PushLiteral()
	{

	}

}