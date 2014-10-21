/** @file
 **
 ** Singleton header and implementation.
 **
 ** A singleton is a class that can be instantiated EXACTLY one time. The class
 ** defined in this header file is ensuring exactly that.
 **
 **
 ***************************************************************************************
 ***************************************************************************************
 ****                                                                               ****
 **** !!! IMPORTANT !!!                                                             ****
 ****                                                                               ****
 ****  As this singleton implementation uses static objects, you must be aware      ****
 ****  that the destruction of the singletons has to be considered to be in a       ****
 ****  random order. DO NOT call any other singletons in your singleton's           ****
 ****  destructor, as there is a  high probability that this will lead to a         ****
 ****  crash (if you are really unlucky this will not manifest for some time).      ****
 ****                                                                               ****
 ***************************************************************************************
 ***************************************************************************************
 **
 **
 ** Usage:
 **
 **  To create a singleton object, you can use the SINGLETON macro to get you
 **  started. Simply pass the name of the class as first, and any constructor
 **  parameters as subsequent parameters (or use void for no parameters).
 **
 **  Example:
 **    SINGLETON(TestClass, void)
 **    ->  class TestClass : public Singleton<TestClass> {
 **          private:
 **            TestClass();
 **            friend class Singleton<className>;
 **
 **    SINGLETON(TestClass, int a, int b)
 **    ->  class TestClass : public Singleton<TestClass> {
 **          private:
 **            TestClass(int a, int b);
 **            friend class Singleton<className>;
 **
 **  Alternatively you may define the class by hand, by deriving it from the
 **  singleton template class (see example). Make sure that all constructors
 **  are private. You also need to add the singleton template as friend, as
 **  otherwise it won't be able to access your private constructor.
 **
 */

#ifndef __SINGLETON_H__
#define __SINGLETON_H__


/*------------------------------------------------------------------------------------------------*/

template <class T>
class Singleton {
  protected:
	Singleton() {}

  private:
	Singleton(const Singleton&) {}

public:
	virtual ~Singleton() {}
	static T& getInstance();

};


/*------------------------------------------------------------------------------------------------*/

template <class T>
T& Singleton<T>::getInstance() {
	static T instance;
	return instance;
}


#endif /* __SINGLETON_H__ */
