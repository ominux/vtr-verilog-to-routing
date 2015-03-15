# C Coding Standards for VTR #

## Naming ##

1. Variable Prefixes:
  * Global variables shall be prefixed with ‘g\_’
  * Enumeration types shall be prefixed with ‘e\_’
  * Typedef types shall be prefixed with ‘t\_’
  * Struct types shall be prefixed with ‘s\_’

2. Enumerated constants, #define, and #define macros shall be written ALL CAPS.

3. Enumerated constants shall have a common prefix, and when appropriate an ending entry.
```

enum e_interconnect {
INTERC_COMPLETE,
INTERC_DIRECT,
INTERC_MUX,
INTERC_MAX_NUM}; ```

## File Organization ##
5. Functions Declaration:
  * Functions used by other source files shall be declared in the '.h' file.
  * Functions used only by the local source shall be declared at the top of the '.c' file and shall be of type 'static'.

6. Typedefs shall be located in a single list at the top of the '.c' file.

7. Variables that are local to a '.c' file shall use the 'static' keyword to prevent populating the global namespace.

## Comments ##
8. Comments shall be provided for:
  * Source Files: a broad overview at the beginning
  * Functions: an overview and description of each parameter
  * Data Structures: an overview and description of each member variable
  * Enumeration:  an overview description and if helpful, a description of each value
  * Global variables: describing their function and scope (used in just packing, placement, etc.)

9. Comments shall always precede the code described, i.e. before function and structure definitions.

## Style ##

10. Global variables should be used very sparingly.  They are best used to share information across multiple separate modules.
```

/* Global architecture information */
t_arch * g_arch; ```

11. VTR uses auto-formatting of code.  See [Auto Formatting](AutoFormatting.md).