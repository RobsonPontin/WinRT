/*
 * Some exercises related to coroutines and async programming with
 * C++/WinRT
 *
 * References:
 * 1. https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/concurrency
 * 2. https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/std-cpp-data-types#standard-arrays-and-vectors
 */

 /*
  * Key concepts:
  * - IAsyncAction	: simply execute an action without any result object, nor ongoing progress
  * - IAsyncActionWithProgress<TProgress> : simply execute an action but do report back to the caller, still does not have a result object.
  * - IAsyncOperation<TResult> : used when there is the need to return an object type
  * - IAsyncOperationWithProgress<TResult, TProgress> : when need to return an object type with progress
 */

#pragma once

#include <iostream>
#include <ppltasks.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Syndication.h>
#include <winrt/Windows.System.Threading.h>

#include "ISample.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Syndication;
using namespace Windows::System::Threading;

// Function prototypes
void ProcessFeedAsync();
concurrency::task<std::wstring> RetrieveFirstTitleAsync();
void StartResetTimer();

struct ConcurrencyTest : ISample
{
	// Example 1 - For Windows Runtime Type
	SyndicationFeed syndFeedResult;
	void ProcessFeedAsync()
	{
		Uri rssFeedUri{ L"https://blogs.windows.com/feed" };
		SyndicationClient syndClient;

		// Option 1 - use co_await 
		//SyndicationFeed syndFeed
		//{
		//	// initiate async call and then returns control to the caller,
		//	// until the caller uses get() to finish executing the coroutine
		//	co_await syndClient.RetrieveFeedAsync(rssFeedUri)
		//};

		// Option 2 - use delegate types with completed and progress events
		auto result_progress = syndClient.RetrieveFeedAsync(rssFeedUri);

		result_progress.Progress([this](
			IAsyncOperationWithProgress<SyndicationFeed,
			RetrievalProgress> const& /* sender */,
			RetrievalProgress const& args)
			{
				uint32_t bytes_received = 0;
				bytes_received = args.BytesRetrieved;

			});

		result_progress.Completed([this](
			IAsyncOperationWithProgress<SyndicationFeed,
			RetrievalProgress> const& sender,
			AsyncStatus const /* asyncStatus */)
			{
				syndFeedResult = sender.GetResults();
				m_isFeedRetrieved = true;
			});


		// Print it
		for (auto const& syndItem : syndFeedResult.Items())
		{
			std::wcout << syndItem.Title().Text().c_str() << std::endl;
		}
	}

	// Example 2 - Returning non-Windows Runtime Type
	concurrency::task<std::wstring> RetrieveFirstTitleAsync()
	{
		return concurrency::create_task([]
			{
				Uri rssFeed{ L"https://blogs.windows.com/feed" };
				SyndicationClient syndClient;
				SyndicationFeed syndFeed
				{
					syndClient.RetrieveFeedAsync(rssFeed).get()
				};

				std::wcout << "Feed has been retrieved!" << std::endl;

				std::wstring wString{ syndFeed.Title().Text() };

				// Example 3 can handle it to ensure thread can finish properly
				//m_isFeedRetrieved = true;

				return wString;
			});
	}

	// Example 3 - dealing with ThreadPoolTimer
	void MyTimerElapsedHandler(ThreadPoolTimer timer) // TODO: noexcept?
	{
		auto result = RetrieveFirstTitleAsync().get();

		timer.Cancel();

		std::wcout << "Timer has been cancelled! " << std::endl;

		m_isFeedRetrieved = true; // once everything is satisfyied we break the main loop
	}

	void StartResetTimer()
	{
		TimerElapsedHandler timerHandler([this](const ThreadPoolTimer& timer)
			{
				MyTimerElapsedHandler(timer);
			});

		std::chrono::duration<int, std::milli> tDuration{ 2000 };

		auto poolTimer =
			ThreadPoolTimer::CreatePeriodicTimer(timerHandler, tDuration);

		std::wcout << "Timer has Started!" << std::endl;
	}

	void Run()
	{
		PrintIt("Concurrency test started!");

		// Example 1
		//ProcessFeedAsync();

		// Example 2
		//auto taskGetTitle = RetrieveFirstTitleAsync();

		// Example 3
		StartResetTimer();

		// ...
		// Do other work here while async is being processed
		// ...

		//rslt.get(); // blocking: call get() to block and finish coroutine, then see result

		
		while (!m_isFeedRetrieved)
		{
		}

		//auto dd = taskGetTitle.get();

		PrintIt("Concurrency test finished!");
	}

private:
	bool m_isFeedRetrieved{ false };
};

