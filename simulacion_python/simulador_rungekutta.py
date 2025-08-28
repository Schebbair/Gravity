import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

pos_escala = 1e6   # 1 unidad = 1 millón km
masa_escala = 1e24 # masas en 10^24 kg

# Datos planetas (masa, distancia, velocidad)
datos_planetas = {
    "Mercurio": (0.330, 57.9, 47.87),
    "Venus":    (4.87, 108.2, 35.02),
    "Tierra":   (5.97, 149.6, 29.78),
    "Marte":    (0.642, 227.9, 24.077)
}

#en 10^24 kg
masa_sol = 1989e30 / masa_escala

# Calcula G efectivo basado en la Tierra
masa_tierra = datos_planetas["Tierra"][0]
dist_tierra = datos_planetas["Tierra"][1]
vel_tierra = datos_planetas["Tierra"][2]
G = (vel_tierra**2.3 * dist_tierra) / masa_sol


class Cuerpo:
    def __init__(self, masa, posicion, velocidad):
        self.masa = masa
        self.posicion = np.array(posicion, dtype=float)
        self.velocidad = np.array(velocidad, dtype=float)

def calcular_aceleraciones(cuerpos):
    n = len(cuerpos)
    aceleraciones = [np.zeros(2) for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if i != j:
                diferencia = cuerpos[j].posicion - cuerpos[i].posicion
                distancia = np.linalg.norm(diferencia)
                if dist > 1e-5:
                    direccion_fuerza = diferencia / distancia
                    aceleraciones[i] += G * cuerpos[j].masa / distancia**2 * direccion_fuerza
    return aceleraciones

def rk4_paso(cuerpos, dt):
    n = len(cuerpos)
    posicion = np.array([c.posicion for c in cuerpos])
    velocidad = np.array([c.velocidad for c in cuerpos])
    masas = np.array([c.masa for c in cuerpos])

    def derivadas(posiciones, velocidades):
        temp_cuerpos = [Cuerpo(masas[i], posiciones[i], velocidades[i]) for i in range(n)]
        return np.array(calcular_aceleraciones(temp_cuerpos))

    a1 = derivadas(posicion, velocidad)
    k1_posicion = velocidad
    k1_velocidad = a1

    a2 = derivadas(posicion + 0.5 * dt * k1_posicion, velocidad + 0.5 * dt * k1_velocidad)
    k2_posicion = velocidad + 0.5 * dt * k1_velocidad
    k2_velocidad = a2

    a3 = derivadas(posicion + 0.5 * dt * k2_posicion, velocidad + 0.5 * dt * k2_velocidad)
    k3_posicion = velocidad + 0.5 * dt * k2_velocidad
    k3_velocidad = a3

    a4 = derivadas(posicion + dt * k3_posicion, velocidad + dt * k3_velocidad)
    k4_posicion = velocidad + dt * k3_velocidad
    k4_velocidad = a4

    posicion += (dt / 6) * (k1_posicion + 2*k2_posicion + 2*k3_posicion + k4_posicion)
    velocidad += (dt / 6) * (k1_velocidad + 2*k2_velocidad + 2*k3_velocidad + k4_velocidad)

    for i in range(n):
        cuerpos[i].posicion = posicion[i]
        cuerpos[i].velocidad = velocidad[i]

factor_velocidad = 1.5

cuerpos = [Cuerpo(masa_sol, [0, 0], [0, 0])]

# Agregar planetas interiores con velocidad perpendicular a posición
for nombre, (masa, dist, vel) in datos_planetas.items():
    posicion = np.array([dist, 0])
    vector_velocidad = np.array([0, vel * factor_velocidad])
    cuerpos.append(Cuerpo(masa, posicion, vector_velocidad))

dt = 0.01

fig, ax = plt.subplots(figsize=(8,8))
ax.set_xlim(-300, 300)
ax.set_ylim(-300, 300)
ax.set_aspect('equal')
puntos, = ax.plot([], [], 'o')

def animar(frame):
    rk4_paso(cuerpos, dt)
    x = [c.posicion[0] for c in cuerpos]
    y = [c.posicion[1] for c in cuerpos]
    puntos.set_data(x, y)
    return puntos,

ani = FuncAnimation(fig, animar, frames=2000, interval=20, blit=True)
plt.show()