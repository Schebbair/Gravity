# Simulador de Gravedad

Este proyecto es un simulador básico de gravedad con cuerpos orbitando bajo la influencia gravitacional mutua. Está pensado para aprender y mejorar habilidades en programación, matemáticas y física. Hecho tanto en Python como en C++.

## Requisitos

### Python
- Python 3.x
- Numpy
- Matplotlib

### C++
- CMake >= 3.20
- GCC/C++
- GLFW, GLAD

## Ejecutar

### Python
1. Clonar el repositorio
2. Ejecutar la versión básica con Euler.
3. Ejecutar la versión avanzada con Runge-Kutta.
4. Visualizar la simulación animada.

### C++
1. Clonar el repositorio
2. cd simulacion_c++
3. mkdir build
4. cd build
5. cmake ..
6. make
7. ./simulador | ./simulador_verlet


---

## Fundamento físico

La simulación se basa en la **ley de gravitación universal de Newton**, que establece que dos cuerpos con masas \$m_1\$ y \$m_2\$, separados por una distancia \$r\$, se atraen con una fuerza:

$$
F = G \frac{m_1 m_2}{r^2}
$$

donde \$G\$ es la constante gravitacional. Esta fuerza provoca una aceleración en cada cuerpo que altera su velocidad y posición en el espacio.

Mi objetivo con este proyecto es calcular cómo evolucionan las posiciones y velocidades de los cuerpos en el tiempo bajo estas fuerzas.

---

## Fase 1: Simulación básica con integración Euler

### Teoría del método Euler

El método de **Euler** es el método numérico más simple para resolver ecuaciones diferenciales ordinarias (como las de movimiento):

$$
\vec{v}_{t+\Delta t} = \vec{v}_t + \vec{a}_t \Delta t
$$

$$
\vec{x}_{t+\Delta t} = \vec{x}_t + \vec{v}_t \Delta t
$$

Donde \$\vec{a}_t\$ es la aceleración calculada en el tiempo \$t\$, \$\vec{v}\$ es la velocidad, y \$\vec{x}\$ la posición.

- **Ventajas:** Fácil de implementar, computacionalmente barato.
- **Desventajas:** Poco preciso, puede generar errores acumulativos grandes y movimientos no realistas en simulaciones prolongadas.

En esta fase usé este método para entender la dinámica básica y la estructura del código.

---

## Fase 2: Mejora con método Runge-Kutta de cuarto orden (RK4)

### Teoría del método Runge-Kutta 4

El método **Runge-Kutta de cuarto orden** es un método numérico avanzado para resolver ecuaciones diferenciales que calcula varias estimaciones de la pendiente en cada paso para obtener un resultado mucho más preciso.

La idea es evaluar la aceleración y velocidad en cuatro puntos intermedios dentro del intervalo \$\Delta t\$, y combinarlos para aproximar la evolución:

$$
\vec{x}_{t+\Delta t} = \vec{x}_t + \frac{\Delta t}{6} (k_1 + 2k_2 + 2k_3 + k_4)
$$

donde cada \$k_i\$ es una estimación de la derivada (velocidad o aceleración) en puntos intermedios.

- **Ventajas:** Mucho más preciso y estable para pasos de tiempo más grandes.
- **Desventajas:** Requiere más cálculos por paso.

Esta implementación mejora la estabilidad y realismo de las órbitas, incluyendo la interacción entre varios cuerpos (por ejemplo, Sol, Venus, Tierra y Marte).

---

## Fase 3: Mejora con método Verlet (C++)

### Teoría del método Verlet

El método de Verlet es un esquema numérico utilizado para integrar ecuaciones de movimiento en mecánica clásica. Es especialmente útil en sistemas donde las fuerzas dependen de la posición (como la gravedad) y se desea conservar energía de manera precisa a largo plazo.

### Idea del método

El método Verlet calcula la nueva posición usando la posición actual y la posición anterior:

$$
\vec{r}(t+\Delta t) = 2 \vec{r}(t) - \vec{r}(t-\Delta t) + \vec{a}(t) \Delta t^2
$$

- La nueva posición depende solo de las posiciones y la aceleración.
- Es simétrico en el tiempo, lo que ayuda a conservar la energía del sistema.

### Ventajas
- Más estable que el método de Euler simple.
- Conserva la energía a largo plazo en sistemas cerrados.
- Muy usado en simulaciones de órbitas planetarias y dinámica molecular.


---

## Próximos pasos futuros

- Evolucionar el simulador hacia objetos con campos gravitacionales extremos, como **agujeros negros**, explorando conceptos de relatividad general.

