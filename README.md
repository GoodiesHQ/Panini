# Panini

**P**arse **A**nd i**N**tegrate  **INI** Files

Panini is a very simple, header-only, gluten-free INI-format configuration file parser.


### Example INI:
```ini
[general]
; some general information about the file
interface = eth0
connections = 100

[testing]
; some test parameters
my_double = 7.3
my_int = -21
```

### Example CPP:

```cpp
#include <string>
#include <panini.hpp>


int main(int argc, char **argv){
    if(argc != 2){
        std::cerr << "Usage: " << argv[0] << " [config-file.ini]\n";
        return -1;
    }

    try {
	    // Read and parse the INI file
        Panini p(argv[1]);
		
		// Defaults to std::string (copy)
        auto interface = p.get("general", "interface");
		
		// Provide another type via a template
        unsigned connections = p.get<unsigned>("general", "connections");
        double my_double = p.get<double>("testing", "my_double");
        int my_int = p.get<int>("testing", "my_int");
    }catch(const std::runtime_error &e){
        std::cerr << e.what();
        return -1;
    }
    return 0;
}
```
