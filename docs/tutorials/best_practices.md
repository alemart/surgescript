Best Practices
==============

- Read the [Components](/tutorials/components) section. It's crucial that you understand it.
- Follow the golden rule: objects should **not** mess with each others' internals!
    - Objects should **not** change other objects' variables or states directly (won't allow it!)
    - Objects should define [functions](/tutorials/functions) that can be operated upon by the external world (API)
- Use *camelCase* names for both variables and functions.
- Use *4 spaces* when indenting your code.
- Combine related [plugins](/tutorials/plugins) into a single package.
    - Do **not** pollute the global namespace.
- Read the [SurgeScript documentation](/) often.