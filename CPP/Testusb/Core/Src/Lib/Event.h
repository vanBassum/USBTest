#ifndef _EVENT_H_
#define _EVENT_H_

//Would advise against this...
//#define BIND(func)	Bind(this, &func)

#include <list>



/// <summary>
/// The abstract overcoupling object to be used by all implementations of the event handler.
/// </summary>
template<typename ...Args>
class EventHandler
{
public:
	EventHandler(){}
	virtual ~EventHandler(){}
	/// <summary>
	/// Used to trigger the event.
	/// </summary>
	/// <param name="args...">Arguments of the event.</param>
	virtual void Invoke(Args... args) = 0;
};


/// <summary>
/// The implementation of an eventhander for methods.
/// </summary>
template<typename T, typename ...Args>
class EventHandlerMethod : public EventHandler<Args...>
{
private:
	void(T::*method)(Args...);
	T* methodInstance;

public:

	/// <summary>
	/// Constructor.
	/// </summary>
	/// <param name="instance">The instance that contains the method to be called.</param>
	/// <param name="memberFunctionToCall">The method to be called.</param>
	EventHandlerMethod<T, Args...>(T* instance, void(T::*memberFunctionToCall)(Args...))
	{
		method = memberFunctionToCall;
		methodInstance = instance;
	}

	/// <summary>
	/// Used to trigger the event.
	/// </summary>
	/// <param name="args...">Arguments of the event.</param>
	void Invoke(Args... args)
	{
		(methodInstance->*method)(args...);
	}

};

/// <summary>
/// The implementation of an eventhander for functions.
/// </summary>
template<typename ...Args>
class EventHandlerFunc : public EventHandler<Args...>
{
private:
	void(*func)(Args...);

public:

	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="functionCall">Pointer to function that should be invoked ones the event is fired.</param>
	EventHandlerFunc<Args...>( void(*functionCall)(Args...))
	{
		func = functionCall;
	}

	/// <summary>
	/// Used to trigger the event.
	/// </summary>
	/// <param name="args...">Arguments of the event.</param>
	void Invoke(Args... args)
	{
		(*func)(args...);
	}

};


/// <summary>
/// </summary>
/// <example>
/// <code>
/// class MyClass
/// {
/// public:
///		MyClass()
///		{
///			SomeEvent.Bind(this, &MyClass::OnEvent);
///		}
/// 
///		void Trigger(int i)
///		{
///			SomeEvent();
///		}
/// 
///		void OnEvent(int i)
///		{
///			printf("Hello %d", i);
///		}
/// 
///		Event<int> SomeEvent;
/// };
/// 
/// 
/// </code>
/// </example>
template<typename ...Args>
class Event
{
private:
	std::list<EventHandler<Args...>*> handlers;

public:

	~Event()
	{
		for(typename std::list< EventHandler<Args...>* >::iterator iter= handlers.begin(); iter != handlers.end(); ++iter)
		{
			EventHandler<Args...>* pHandler= *iter;

			if(pHandler)
			{
				delete pHandler;
				pHandler= 0;  // just to be consistent
			}
		}
		handlers.clear();
	}

/*	Not possible, dont know the object that contains the method.
	template<typename T>
	void operator+= (void(T::*method)(Args...))
	{

	}
*/

	/// <summary>
	/// Used to bind a method to the event.
	/// </summary>
	/// <example>
	/// <code>
	/// SomeEvent.Bind(this, &MyClass::OnEvent);
	/// </code>
	/// </example>
	/// <param name="methodInstance">The owner of the method</param>
	/// <param name="method">The method to be bound to the event.</param>
	template<typename T>
	void Bind( T* methodInstance, void(T::*method)(Args...))
	{
		handlers.push_back(new EventHandlerMethod<T, Args...>(methodInstance, method));
	}

	/// <summary>
	/// Used to bind a function to the event.
	/// </summary>
	/// <example>
	/// <code>
	/// SomeEvent.Bind(&OnEvent);
	/// </code>
	/// </example>
	/// <param name="function">The function to be bound to the event.</param>
	void Bind(void(*function)(Args...))
	{
		handlers.push_back(new EventHandlerFunc<Args...>(function));
	}


	//TODO: implement events with void parameter.

	//TODO: implement unbinding of events.

	/// <summary>
	/// Used to invoke the event.
	/// </summary>
	/// <param name="args...">Arguments</param>
	void operator()(Args... args)
	{
		//https://stackoverflow.com/questions/1571215/c-iterating-over-a-list-of-a-generic-type
		for(typename std::list< EventHandler<Args...>* >::iterator iter= handlers.begin(); iter != handlers.end(); ++iter)
		{
			EventHandler<Args...>* pHandler= *iter;
			if(pHandler)
			{
				pHandler->Invoke(args...);
			}
		}

	}
};



#endif
