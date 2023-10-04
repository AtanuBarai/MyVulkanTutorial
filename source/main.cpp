
#include "TestVKSupport.h"
#include "Engine.h"

int runHelloTriangleApp()
{
    try
    {
        std::unique_ptr<Engine> app = std::make_unique<Engine>();
        app->run();
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