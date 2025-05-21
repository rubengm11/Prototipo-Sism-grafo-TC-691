import serial
import time
import matplotlib.pyplot as plt
from collections import deque

# Configuración
PUERTO = 'COM5'  # Cambia según tu equipo
BAUDIOS = 9600
ARCHIVO_SALIDA = "sismos_detectados.txt"

# Rango normal
RANGO_X = (-0.3, 1.5)
RANGO_Y = (-1.5, 1)
RANGO_Z = (8.5, 9.0)

# Inicializar serial
arduino = serial.Serial(PUERTO, BAUDIOS)
time.sleep(2)

# Datos recientes (ventana de tiempo)
max_puntos = 100
datos_x = deque([0]*max_puntos, maxlen=max_puntos)
datos_y = deque([0]*max_puntos, maxlen=max_puntos)
datos_z = deque([0]*max_puntos, maxlen=max_puntos)
tiempos = deque([0]*max_puntos, maxlen=max_puntos)

# Configurar gráfica
plt.ion()
fig, ax = plt.subplots()
linea_x, = ax.plot([], [], label='X')
linea_y, = ax.plot([], [], label='Y')
linea_z, = ax.plot([], [], label='Z')

ax.set_ylim(-3, 11)
ax.set_xlim(0, max_puntos)
ax.set_title("Lectura de Aceleración (MPU6050)")
ax.set_xlabel("Muestras")
ax.set_ylabel("Aceleración (aprox. m/s²)")
ax.legend()
plt.tight_layout()

# Abrir archivo de salida
archivo = open(ARCHIVO_SALIDA, "w")
archivo.write("Registro de sismos (Aceleracion X, Y, Z)\n")

try:
    while True:
        if arduino.in_waiting > 0:
            linea = arduino.readline().decode().strip()
            try:
                ax, ay, az = linea.split("\t")
                ax = float(ax)
                ay = float(ay)
                az = float(az)

                # Agregar a la gráfica
                datos_x.append(ax)
                datos_y.append(ay)
                datos_z.append(az)
                tiempos.append(len(tiempos))

                # Actualizar líneas
                linea_x.set_data(range(len(datos_x)), datos_x)
                linea_y.set_data(range(len(datos_y)), datos_y)
                linea_z.set_data(range(len(datos_z)), datos_z)
                #ax.set_xlim(0, len(datos_x))
                fig.canvas.draw()
                fig.canvas.flush_events()

                # Verificar si está fuera del rango
                fuera_x = not (RANGO_X[0] <= ax <= RANGO_X[1])
                fuera_y = not (RANGO_Y[0] <= ay <= RANGO_Y[1])
                fuera_z = not (RANGO_Z[0] <= az <= RANGO_Z[1])

                if fuera_x or fuera_y:
                    print(f"⚠️ ¡Posible sismo detectado! X={ax:.2f}, Y={ay:.2f}, Z={az:.2f}")
                    archivo.write(f"{ax:.2f}, \t {ay:.2f}, \t {az:.2f}\n")

            except ValueError as e:
                #print("⚠️ Dato malformado recibido:", linea)
                print(e)
except KeyboardInterrupt:
    print("\n⛔ Lectura detenida por el usuario.")
    arduino.close()
    archivo.close()
    print("✅ Archivo guardado como:", ARCHIVO_SALIDA)
