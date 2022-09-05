#pragma once
#include "pch.h"
#include "ISample.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace std::chrono_literals;



// 1. Safely accessing the this pointer in a class-member coroutine
struct MyClass :winrt::implements<MyClass, IInspectable>
{
	winrt::hstring m_myHelloString{ L"Hellow World!" };

	IAsyncOperation<winrt::hstring> RetrieveValuesAsync()
	{
		/* 1. Strong Ref - Ensure to keep *this* alive
		 * Ensure to still have access to the raw pointer of "this" class instance even
		 * in case it gets destroyed
		 */

		 /*
		 auto strong_this{ get_strong() };

		 co_await 5s;

		 co_return m_myHelloString;
		 */

		 /* 2. Weak Ref - Maybe keep *this* alive
		  * Differently from strong ref, the weak reference will not prevent the instance
		  * from being destroyed, but at least we can perform a check if we can proceed and get
		  * a strong reference or not
		  */

		auto weak_this{ get_weak() };
		co_await 5s;

		if (auto strong_this{ weak_this.get() })
		{
			co_return m_myHelloString;
		}
		else
		{
			co_return L"";
		}
	}

};

// 2. Safely accessing the this pointer with an event-handling delegate
struct EventSource
{
	winrt::event<EventHandler<int>> m_event;

	void Event(EventHandler<int> const& handler)
	{
		m_event.add(handler);
	}

	void RaiseEvent()
	{
		m_event(nullptr, 0);
	}
};

struct EventRecipient : winrt::implements<EventRecipient, IInspectable>
{
	winrt::hstring m_value{ L"Hello, World!" };

	// Example 1 - lambda function as delegate
	void Register(EventSource& event_source)
	{
		// PROBLEM - Both cases crash would happen:
		// - event_source.Event([&](auto&& ...)
		// - event_source.Event([this](auto&& ...) 

		// SOLUTION
		// - can also get strong directly - event_source.Event([this, strong_this{get_strong()}](auto&& ...)

		event_source.Event([weak_this{ get_weak() }](auto&& ...)
		{
			if (auto strong_this{ weak_this.get() })
			{
				std::wcout << strong_this->m_value.c_str() << std::endl;
			}
		});
	}

	// Example 2 - member function as delegate
	void Register2(EventSource& event_source)
	{
		/*Option 1 - Strong ref : guarantee to keep alive and always
		 * call the delegate function */
		 //event_source.Event({ get_strong(), &EventRecipient::OnEvent});

		 /* Option 2 - Weak ref : only call the delegate function if weak ref
		  * can be resolved to a strong ref */
		  //event_source.Event({ get_weak(), &EventRecipient::OnEvent });

		  /* Option 3 -  winrt::auto_revoke */
		event_source.Event({ get_weak(), &EventRecipient::OnEvent });

	}

	void OnEvent(IInspectable const&, int)
	{
		std::wcout << m_value.c_str() << std::endl;
	}
};


struct WeakReferenceTest : ISample
{
	void Run()
	{
		PrintIt("WeakReferenceTest has started!");

		// 1.
		/*
		auto myClass_instance{ winrt::make_self<MyClass>() };
		auto asyncCall{ myClass_instance->RetrieveValuesAsync() };

		myClass_instance = nullptr; // Simulate instance going out of scope

		winrt::hstring result{ asyncCall.get() };
		std::wcout << result.c_str() << std::endl;
		*/

		// 2.
		EventSource event_source;
		auto event_recipient{ winrt::make_self<EventRecipient>() };
		//event_recipient->Register(event_source);
		event_recipient->Register2(event_source);

		event_recipient = nullptr; // Simulate the event recipient going out of scope.

		event_source.RaiseEvent();

		PrintIt("WeakReferenceTest has finished!");
	}
};
