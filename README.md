# Motor de itinerarios integrados y tarifas

## Integrante

- Miguel Angel Restrepo Martinez (trabajo individual)

## Compilar y ejecutar

```bash
g++ -std=c++17 -Wall -Wextra -o practice main.cpp
./practice
```

## Preguntas para la defensa oral

### 1. ¿Por qué usar una clase base abstracta con funciones virtuales en vez de un `enum` y un `switch`?

`TransportMode` define la interfaz común de los modos de transporte. Sus funciones puras virtuales exigen que cada modo concreto proporcione su tarifa y tipo de servicio. Así, el inventario y el itinerario trabajan mediante referencias a `TransportMode` y el despacho dinámico invoca el comportamiento correcto. Con un `enum` y un `switch`, agregar un modo obligaría a modificar el `switch` central y cada otro lugar que tome decisiones según el modo. La jerarquía concentra el comportamiento específico en la clase correspondiente y facilita extender el sistema.

### 2. ¿Por qué `TransportMode` debe tener un destructor virtual?

`Network` es dueño de los objetos de modos derivados mediante `std::unique_ptr<TransportMode>`. Cuando un objeto derivado se destruye a través de un puntero a la clase base, el destructor virtual garantiza que se ejecuten tanto el destructor derivado como el de la base. Sin ese destructor virtual, borrar mediante el puntero base tiene comportamiento indefinido y no se puede confiar en que se liberen correctamente los recursos de las clases derivadas. El destructor virtual hace segura la propiedad polimórfica.

## Notas de diseño

- `Passenger::fareOn` aplica el patrón Non-Virtual Interface y limita toda tarifa calculada a cero o más.
- El itinerario se ordena por minuto y aplica la regla de integración de 90 minutos y hasta dos transferencias gratuitas.
- Los informes usan algoritmos STL y funciones de política inyectadas, como pide la práctica.
