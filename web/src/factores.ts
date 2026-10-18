export const factores = [
  {
    titulo: "Batería",
    clave: "bateria",
    opciones: [
      {
        valor: "1",
        nombre: "6 V",
      },
      {
        valor: "1.23",
        nombre: "7,4 V",
      },
      {
        valor: "1.4",
        nombre: "8,4 V (cargada)",
      },
    ],
  },
  {
    titulo: "Motores",
    clave: "motores",
    opciones: [
      {
        valor: "0.08,0.3",
        nombre: "Suaves",
      },
      {
        valor: "0.15,0.5",
        nombre: "Normales",
      },
      {
        valor: "0.3,1.0",
        nombre: "Duros",
      },
    ],
  },
  {
    titulo: "Ruedas",
    clave: "ruedas",
    opciones: [
      {
        valor: "0.025",
        nombre: "2,5 cm",
      },
      {
        valor: "0.03",
        nombre: "3 cm",
      },
      {
        valor: "0.035",
        nombre: "3,5 cm",
      },
    ],
  },
  {
    titulo: "Sensor de piso",
    clave: "piso",
    opciones: [
      {
        valor: "900,100,1000",
        nombre: "Normal",
      },
      {
        valor: "150,850,150",
        nombre: "Invertido",
      },
      {
        valor: "700,420,900",
        nombre: "Poco contraste",
      },
    ],
  },
  {
    titulo: "Alrededor del dohyo",
    clave: "pared",
    opciones: [
      {
        valor: "0",
        nombre: "Nada",
      },
      {
        valor: "0.5",
        nombre: "Pared a 15 cm",
      },
    ],
  },
  {
    titulo: "Tipo de rival",
    clave: "rival",
    opciones: [
      {
        valor: "0.8,0.3,0.6",
        nombre: "Normal (300 g)",
      },
      {
        valor: "0.9,0.3,0.9",
        nombre: "Rápido",
      },
      {
        valor: "0.9,0.5,0.6",
        nombre: "Pesado (500 g)",
      },
    ],
  },
  {
    titulo: "Dohyo",
    clave: "dohyo",
    opciones: [
      {
        valor: "0.9,1",
        nombre: "Limpio",
      },
      {
        valor: "0.6,0.69",
        nombre: "Con polvo",
      },
    ],
  },
  {
    titulo: "Llantas",
    clave: "llantas",
    opciones: [
      {
        valor: "0,0",
        nombre: "Nuevas",
      },
      {
        valor: "0.4,0.4",
        nombre: "Gastadas",
      },
      {
        valor: "0.5,0",
        nombre: "Izquierda gastada",
      },
    ],
  },
  {
    titulo: "Señal de START (control remoto)",
    clave: "cortes",
    opciones: [
      {
        valor: "0",
        nombre: "Sin cortes",
      },
      {
        valor: "0.5",
        nombre: "Cortes a veces",
      },
      {
        valor: "2",
        nombre: "Muchos cortes",
      },
    ],
  },
  {
    titulo: "Sensores de enemigo",
    clave: "sensores",
    opciones: [
      {
        valor: "0,0",
        nombre: "Bien",
      },
      {
        valor: "2,0",
        nombre: "Reflejos",
      },
      {
        valor: "0,0.3",
        nombre: "Cegados por el IR del rival",
      },
    ],
  },
  {
    titulo: "Sensores de piso",
    clave: "retardo",
    opciones: [
      {
        valor: "0",
        nombre: "Rápidos",
      },
      {
        valor: "40",
        nombre: "Lentos (40 ms)",
      },
    ],
  },
] as const;
