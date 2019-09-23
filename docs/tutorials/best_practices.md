Best Practices
==============

- Read the [Components](/tutorials/components) section. It's crucial that you understand it.
- Follow the golden rule: objects should **not** mess with each others' internals!
    - Objects should **not** change others' internal variables or states directly (not allowed!)
    - Objects should define [functions](/tutorials/functions) that can be operated upon by the external world (API)
- Use a consistent style, e.g.,
    - Use *camelCase* names for both variables and functions.
    - Use *4 spaces* when indenting your code.
- Combine related [packages](/tutorials/packages) into a single one: don't pollute the global namespace.
- Read the [SurgeScript documentation](/) often.
- **Practice, practice, practice! - and have fun!**