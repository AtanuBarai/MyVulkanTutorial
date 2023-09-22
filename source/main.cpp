
#include "TestVKSupport.h"
#include "HelloTriangleApplication.h"

int runHelloTriangleApp()
{
    HelloTriangleApplication app;
    try
    {
        app.run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main() 
{
    // testEnvironment();
    runHelloTriangleApp();    
    return 0;
}