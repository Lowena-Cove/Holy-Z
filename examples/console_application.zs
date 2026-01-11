class ExampleClass (SuperClass)
{
    func constructor(Param1, Param2)
    {
        int this.example_attribute = 10
        Printl("Constructor called with params")
    }

    func example_method(Param1)
    {
        Printl("Method called")
    }

    static int example_static_attr = 15
    
    static func example_static_method()
    {
        Printl("Static method called")
    }
}

func Main()
{
    Printl("Testing class functionality")
    ExampleClass a = (1, 2)
}