# Slamhaus Documentation

This directory contains the documentation for the Slamhaus project.

## Generating Documentation

To generate the documentation locally, you need to have [Doxygen](https://www.doxygen.nl/index.html) installed.

Run the following command from the project root:

```bash
doxygen Doxyfile
```

This will generate HTML documentation in `docs/html`. Open `docs/html/index.html` in your browser to view it.

## Documentation Standards

-   **Location:** All documentation comments should be placed in the header files (`.hpp`).
-   **Format:** Use Doxygen-style comments (`///` or `/** ... */`).
-   **Coverage:** All public classes, functions, and structs should be documented.
-   **Style:**
    -   Use `@brief` for a short summary.
    -   Use `@param` for function parameters.
    -   Use `@return` for return values.

## Directory Structure

-   `html/`: Generated HTML documentation (ignored by git).
