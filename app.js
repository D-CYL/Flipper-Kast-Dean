// ============================================
// Node.js applicatie die scores ontvangt via de
// seriële poort (Arduino) en doorstuurt naar een API.
//
// Studenten hoeven alleen een regel tekst te sturen via Serial.println:
//   "Naam,Score"
//   of
//   "Naam,Score,MachineId"
//
// Voorbeeld Arduino:
//   Serial.println("Steven,125000");
//   Serial.println("Steven,125000,team_1_flipper");
//
// Deze app zet dit om naar JSON en POST het naar de server.
//
// Auteur: Steven Slaa
// ============================================

import { SerialPort } from "serialport";              // voor seriële communicatie
import { ReadlineParser } from "@serialport/parser-readline"; // om regels te lezen
import readline from "readline";                     // om commando’s van toetsenbord te lezen

// === Instellingen ===
const SERIAL_PORT = process.env.SERIAL_PORT || "COM5"; // verander dit naar jouw Arduino poort (bijv. COM3 op Windows, /dev/ttyUSB0 of /dev/ttyACM0 op Linux/Mac)
const BAUD_RATE = parseInt(process.env.BAUD_RATE || "9600", 10); // baudrate, zelfde als Arduino Serial.begin()
const API_URL = "https://flipperboard.ti.datalabrotterdam.nl/api/scores"; // API waar scores naartoe gaan
const DEFAULT_MACHINE_ID = "team_1_flipper"; // standaard machine ID als die niet wordt meegestuurd
const RELEASE_WINDOW_MS = 15000; // tijd dat poort dicht gaat bij 'u'-commando (voor uploaden code)

// === Log functies (handig voor nette output) ===
const info = (msg) => console.log("[INFO]", msg);
const warn = (msg) => console.warn("[WARN]", msg);
const error = (msg) => console.error("[ERROR]", msg);

// === Variabelen voor seriële poort ===
let port;
let parser;
let isReleasing = false; // geeft aan of we de poort bewust sluiten voor upload

// === Functie om opnieuw te proberen als de poort wegvalt ===
function scheduleReconnect() {
  if (isReleasing) return; // niet opnieuw openen tijdens upload
  const delay = 2000 + Math.random() * 3000; // wacht 2-5 seconden
  info(`Probeer opnieuw te verbinden over ${Math.round(delay)}ms...`);
  setTimeout(openPort, delay);
}

// === Functie om de seriële poort te openen ===
async function openPort() {
  if (port && port.isOpen) return; // al open, niks doen

  info(`Opening serial port ${SERIAL_PORT} @ ${BAUD_RATE}`);

  // Maak de seriële poort aan
  port = new SerialPort({ path: SERIAL_PORT, baudRate: BAUD_RATE, autoOpen: false });

  // Parser zorgt dat we volledige regels krijgen (gescheiden door \n)
  parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));

  // Als er data binnenkomt → roep onLine() aan
  parser.on("data", onLine);

  // Handige events
  port.on("open", () => info("✅ Seriële poort geopend"));
  port.on("error", (err) => warn("Serial error: " + err.message));
  port.on("close", () => {
    warn("Seriële poort gesloten");
    scheduleReconnect(); // probeer opnieuw te verbinden
  });

  // Probeer de poort te openen
  port.open((err) => {
    if (err) {
      warn("Kan poort niet openen: " + err.message);
      scheduleReconnect();
    }
  });
}

// === Handler voor ontvangen regels ===
async function onLine(raw) {
  const line = raw.trim(); // verwijder spaties en newline
  info("Ontvangen: " + line);

  // Splits de regel op komma's
  const parts = line.split(",").map((p) => p.trim());

  // Controleer of er 2 of 3 velden zijn
  if (parts.length !== 2 && parts.length !== 3) {
    badFormat(line, `Verwacht 2 of 3 velden, vond ${parts.length}`);
    return;
  }

  // Haal velden uit de array
  const [player_name, scoreStr, machine_id = DEFAULT_MACHINE_ID] = parts;
  const score = parseInt(scoreStr, 10);

  // Validatie
  if (!player_name) return badFormat(line, "Naam is leeg");
  if (isNaN(score)) return badFormat(line, `Score is geen getal ('${scoreStr}')`);
  if (!machine_id) return badFormat(line, "Machine ID is leeg");

  // Bouw JSON object
  const payload = { player_name, score, machine_id };

  // Verstuur naar API
  await sendPayload(payload);
}

// === Als de invoer fout is, toon uitleg en voorbeelden ===
function badFormat(line, reason) {
  warn(`❌ Ongeldig bericht: ${reason} — "${line}"`);
  warn("Correcte voorbeelden:");
  warn("  Steven,125000");
  warn("  Steven,125000,team_1_flipper");
}

// === Functie om data naar de API te sturen ===
async function sendPayload(payload) {
  try {
    const res = await fetch(API_URL, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });

    if (res.ok) {
      info(`✅ Score verzonden: ${JSON.stringify(payload)}`);
    } else {
      const text = await res.text();
      error(`❌ Server error: ${res.status} ${text}`);
    }
  } catch (err) {
    error("❌ Netwerkfout bij verzenden: " + err.message);
  }
}

// === CLI commando's (van toetsenbord) ===
const rl = readline.createInterface({ input: process.stdin, output: process.stdout });
rl.on("line", async (cmd) => {
  if (cmd.trim().toLowerCase() === "u") {
    // 'u' = release poort tijdelijk voor upload
    if (!port || !port.isOpen) {
      info("Poort is al gesloten.");
      return;
    }
    info(`🔓 Poort vrijgegeven voor ${RELEASE_WINDOW_MS / 1000}s (Arduino upload).`);
    isReleasing = true;
    port.close(() => {
      setTimeout(() => {
        isReleasing = false;
        openPort(); // daarna poort weer openen
      }, RELEASE_WINDOW_MS);
    });
  }
});

// === Start de app ===
openPort();
info("App gestart. Typ 'u' + ENTER om poort tijdelijk te sluiten voor Arduino uploads.");
info("             Typ CTRL + 'c' om applicatie te sluiten.");
