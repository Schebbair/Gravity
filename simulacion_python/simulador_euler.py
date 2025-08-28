import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

#Constante Gravitacional
G = 6.67430e-11

class Cuerpo:
    def __init__(self, masa, posicion, velocidad):
        self.masa = masa
        self.posicion = np.array(posicion, dtype=float)
        self.velocidad = np.array(velocidad, dtype=float)
        self.fuerza = np.zeros(2, dtype=float)


#Función para calcular la fuerza gravitatoria que ejercen entre sí dos cuerpos
def calcular_fuerza(c1,c2):
    vector_distancia = c2.posicion-c1.posicion
    distancia = np.linalg.norm(vector_distancia)
    if distancia == 0:
        return np.zeros(2)
    magnitud_fuerza = G * c1.masa * c2.masa / distancia**2
    vector_fuerza = magnitud_fuerza * (vector_distancia/distancia)
    return vector_fuerza

#Función para actualizar velocidad y posición
def actualizar (cuerpos, dt):
    #primero reiniciar la fuerza
    for c in cuerpos:
        c.fuerza[:] = 0
    
    #calcular fuerza
    for i in range(len(cuerpos)):
        for j in range(i+1, len(cuerpos)):
            f = calcular_fuerza(cuerpos[i], cuerpos[j])
            cuerpos[i].fuerza += f
            cuerpos[j].fuerza -= f
    
    #actualizar velocidad y posición
    for c in cuerpos:
        aceleracion = c.fuerza / c.masa
        c.velocidad += aceleracion * dt
        c.posicion += c.velocidad * dt


#-----------------------------------------------------------------------------------------------------

# (masa, distancia, velocidad)
c1 = Cuerpo(10000000000000000, [0,0], [0,0]) #cuerpo central

#cuerpo orbitante
c2 = Cuerpo(10, [50,0], [0,np.sqrt(G * c1.masa / 50)]) #la velocidad ha de ser perpendicular a r
c3 = Cuerpo(1000, [80,0], [0, np.sqrt(G * c1.masa / 80)])
c4 = Cuerpo(1000000000, [100,0], [0, np.sqrt(G * c1.masa / 100)])

dt = 0.1 # paso del tiempo
fig, ax = plt.subplots()
ax.set_xlim(-100, 100)
ax.set_ylim(-100, 100)
scat = ax.scatter([], [])
cuerpos = [c1,c2,c3,c4]

#Función para animación
def animar(frame):
    actualizar(cuerpos, dt)
    posiciones = [c.posicion for c in cuerpos]
    scat.set_offsets(posiciones)
    return [scat]

ani = FuncAnimation(fig, animar, frames = 600, interval = 20, blit = True)
plt.show()