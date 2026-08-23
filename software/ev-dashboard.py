"""
EV Dashboard - Pygame Version
"""

import pygame
import math
import time
import threading
import json
import socket
from datetime import datetime
import os

# ===== SETTINGS =====
UDP_PORT = 5005
FPS = 60

# Screen settings
SCREEN_WIDTH = 1024
SCREEN_HEIGHT = 600

# ===== COLORS =====
BLACK = (8, 12, 24)
DARK_BG = (11, 17, 30)
CARD_BG = (17, 24, 39, 200)
CYAN = (34, 211, 238)
GREEN = (52, 211, 153)
GOLD = (251, 191, 36)
RED = (248, 113, 113)
WHITE = (241, 245, 249)
GRAY = (100, 116, 139)
LIGHT_GRAY = (148, 163, 184)
DARK_GRAY = (30, 41, 59)


class TelemetryData:
    def __init__(self):
        self.speed = 0.0
        self.temperature = 0.0
        self.vibration = 0.0
        self.distance = 0
        self.gear = 'P'
        self.signal = 4

    def from_json(self, json_str):
        try:
            data = json.loads(json_str)
            self.speed = float(data.get('speed', 0.0))
            self.temperature = float(data.get('temperatureData', 0.0))
            self.vibration = float(data.get('vibrationData', 0.0))
            self.distance = int(data.get('distance', 0))

            s = self.speed
            if s < 1:
                self.gear = 'P'
            elif s < 20:
                self.gear = '1'
            elif s < 50:
                self.gear = '2'
            elif s < 90:
                self.gear = '3'
            elif s < 130:
                self.gear = '4'
            else:
                self.gear = '5'
            return True
        except (json.JSONDecodeError, ValueError, TypeError):
            return False


class UDPServer:
    def __init__(self, port=5005):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('0.0.0.0', port))
        self.sock.settimeout(0.1)
        self.last_data = None
        self.last_received = time.time()

    def get_data(self):
        try:
            data, addr = self.sock.recvfrom(1024)
            json_str = data.decode('utf-8')
            self.last_data = json.loads(json_str)
            self.last_received = time.time()
            return self.last_data
        except socket.timeout:
            return None
        except (json.JSONDecodeError, UnicodeDecodeError):
            return None

    def close(self):
        self.sock.close()


class EVDashboard:
    def __init__(self):
        pygame.init()
        pygame.display.set_caption("EV SIVA Dashboard")

        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        self.clock = pygame.time.Clock()
        self.running = True

        self._load_fonts()
        self._load_images()

        self.data = TelemetryData()
        self.data_lock = threading.Lock()
        self.connected = False
        self.simulate_mode = False

        self.udp = UDPServer(UDP_PORT)
        self.udp_thread = threading.Thread(target=self._udp_loop, daemon=True)
        self.udp_thread.start()

        self.pulse = 0
        self.glow_phase = 0

        print("\nEV SIVA Dashboard Started!")
        print(f"   Listening on UDP port {UDP_PORT}")
        print("   Press ESC or Q to quit")
        print("   Press F for fullscreen")
        print("=" * 50)

    def _load_fonts(self):
        try:
            self.font_small = pygame.font.Font(None, 18)
            self.font_regular = pygame.font.Font(None, 24)
            self.font_medium = pygame.font.Font(None, 32)
            self.font_large = pygame.font.Font(None, 48)
            self.font_huge = pygame.font.Font(None, 96)
            self.font_mega = pygame.font.Font(None, 120)
        except pygame.error:
            self.font_small = pygame.font.SysFont('Arial', 18)
            self.font_regular = pygame.font.SysFont('Arial', 24)
            self.font_medium = pygame.font.SysFont('Arial', 32)
            self.font_large = pygame.font.SysFont('Arial', 48)
            self.font_huge = pygame.font.SysFont('Arial', 96)
            self.font_mega = pygame.font.SysFont('Arial', 120)

    def _load_images(self):
        """Load all icon images"""
        self.icons = {}
        self.car_image = None

        # Icon names and their display names
        icon_files = {
            'logo': 'logo.png',
            'temp': 'temp_icon.png',
            'vibration': 'vibration_icon.png',
            'distance': 'distance_icon.png',
            'connected': 'connected_icon.png',
            'disconnected': 'disconnected_icon.png',
            'car': 'car.png'
        }

        # Create icons directory if it doesn't exist
        if not os.path.exists('icons'):
            os.makedirs('icons')
            print("   Created 'icons' folder - please add icon images")

        # Try to load each icon
        for key, filename in icon_files.items():
            # Check multiple paths
            paths = [
                f'icons/{filename}',
                filename,
                f'assets/{filename}',
                f'assets/icons/{filename}'
            ]

            loaded = False
            for path in paths:
                if os.path.exists(path):
                    try:
                        img = pygame.image.load(path).convert_alpha()
                        # Scale icons to appropriate sizes
                        if key == 'car':
                            self.car_image = pygame.transform.scale(img, (350, 150))
                        elif key == 'logo':
                            self.icons[key] = pygame.transform.scale(img, (48, 48))
                        else:
                            self.icons[key] = pygame.transform.scale(img, (24, 24))
                        print(f"   Loaded: {path}")
                        loaded = True
                        break
                    except pygame.error:
                        continue

            if not loaded and key != 'car':
                # Create fallback colored circle
                self.icons[key] = self._create_fallback_icon(key)
                print(f"   Using fallback for: {key}")

        # Create fallback car if needed
        if self.car_image is None:
            self.car_image = self._create_fallback_car()
            print("   Using fallback car shape")

    def _create_fallback_icon(self, name):
        """Create a simple colored circle as fallback icon"""
        surf = pygame.Surface((24, 24), pygame.SRCALPHA)

        colors = {
            'logo': CYAN,
            'temp': (255, 100, 50),
            'vibration': (255, 200, 50),
            'distance': (50, 200, 255),
            'connected': GREEN,
            'disconnected': RED
        }

        color = colors.get(name, WHITE)
        pygame.draw.circle(surf, color, (12, 12), 10)
        pygame.draw.circle(surf, (255, 255, 255, 50), (12, 12), 10, 2)

        # Add simple text
        if name == 'logo':
            font = pygame.font.Font(None, 16)
            text = font.render("EV", True, WHITE)
            text_rect = text.get_rect(center=(12, 12))
            surf.blit(text, text_rect)
        elif name == 'temp':
            font = pygame.font.Font(None, 14)
            text = font.render("T", True, WHITE)
            text_rect = text.get_rect(center=(12, 12))
            surf.blit(text, text_rect)
        elif name == 'vibration':
            font = pygame.font.Font(None, 14)
            text = font.render("~", True, WHITE)
            text_rect = text.get_rect(center=(12, 12))
            surf.blit(text, text_rect)
        elif name == 'distance':
            font = pygame.font.Font(None, 14)
            text = font.render("◉", True, WHITE)
            text_rect = text.get_rect(center=(12, 12))
            surf.blit(text, text_rect)

        return surf

    def _create_fallback_car(self):
        """Create a simple car shape as fallback"""
        car_surface = pygame.Surface((350, 150), pygame.SRCALPHA)

        # Car body
        car_body = [
            (50, 80), (50, 40), (100, 20), (180, 20),
            (240, 40), (300, 40), (320, 60), (320, 80)
        ]
        pygame.draw.polygon(car_surface, (34, 211, 238, 180), car_body, 0)
        pygame.draw.polygon(car_surface, (34, 211, 238, 255), car_body, 2)

        # Windows
        pygame.draw.polygon(car_surface, (100, 200, 255, 100),
                           [(100, 35), (180, 25), (180, 35), (100, 45)], 0)
        pygame.draw.polygon(car_surface, (100, 200, 255, 100),
                           [(190, 30), (240, 45), (240, 55), (190, 40)], 0)

        # Wheels
        pygame.draw.circle(car_surface, (30, 30, 40), (80, 90), 20)
        pygame.draw.circle(car_surface, (50, 50, 60), (80, 90), 15)
        pygame.draw.circle(car_surface, (30, 30, 40), (280, 90), 20)
        pygame.draw.circle(car_surface, (50, 50, 60), (280, 90), 15)

        # Headlights
        pygame.draw.circle(car_surface, (255, 255, 200, 150), (305, 50), 8)
        pygame.draw.circle(car_surface, (255, 255, 200, 150), (305, 70), 8)
        pygame.draw.circle(car_surface, (255, 50, 50, 150), (55, 50), 8)
        pygame.draw.circle(car_surface, (255, 50, 50, 150), (55, 70), 8)

        return car_surface

    def _udp_loop(self):
        while self.running:
            data = self.udp.get_data()
            if data:
                with self.data_lock:
                    self.data.from_json(json.dumps(data))
                    self.connected = True
                    self.simulate_mode = False
            elif time.time() - self.udp.last_received > 2.0:
                if not self.simulate_mode:
                    self.simulate_mode = True
                    print("   No data, simulation mode")

            if self.simulate_mode:
                self._simulate_data()

            time.sleep(0.02)

    def _simulate_data(self):
        with self.data_lock:
            speed = self.data.speed
            if speed > 10:
                delta = (0.8 - (speed / 200.0) * 0.8) * 2.0
            else:
                delta = 0.5 + (speed / 200.0) * 4.0

            self.data.speed = max(0.0, min(200.0, speed + delta))
            self.data.temperature = 75.0 + (self.data.speed / 200.0) * 30.0
            self.data.vibration = (self.data.speed / 200.0) * 15.0
            self.data.distance = int(max(30, 500 - self.data.speed * 2 + (time.time() * 20) % 100))

            s = self.data.speed
            if s < 1:
                self.data.gear = 'P'
            elif s < 20:
                self.data.gear = '1'
            elif s < 50:
                self.data.gear = '2'
            elif s < 90:
                self.data.gear = '3'
            elif s < 130:
                self.data.gear = '4'
            else:
                self.data.gear = '5'

    def _draw_background(self):
        for y in range(SCREEN_HEIGHT):
            factor = y / SCREEN_HEIGHT
            r = int(8 + (15 - 8) * factor)
            g = int(12 + (26 - 12) * factor)
            b = int(24 + (48 - 24) * factor)
            pygame.draw.line(self.screen, (r, g, b), (0, y), (SCREEN_WIDTH, y))

    def _draw_top_bar(self):
        bar_rect = pygame.Rect(20, 15, SCREEN_WIDTH - 40, 55)
        surf = pygame.Surface((bar_rect.width, bar_rect.height), pygame.SRCALPHA)
        surf.fill((17, 24, 39, 180))
        self.screen.blit(surf, (bar_rect.x, bar_rect.y))
        pygame.draw.rect(self.screen, (34, 211, 238, 30), bar_rect, 1, border_radius=12)

        # Center Y for vertical alignment
        center_y = bar_rect.centery - 10  # Adjust for text baseline

        # Logo icon - left aligned, vertically centered
        icon_x = 35
        if 'logo' in self.icons:
            icon_rect = self.icons['logo'].get_rect(center=(icon_x + 12, center_y + 2))
            self.screen.blit(self.icons['logo'], icon_rect)
            text_x = icon_x + 55
        else:
            text_x = 40

        # Brand - left aligned, vertically centered
        brand = self.font_large.render("EV SIVA", True, CYAN)
        brand_rect = brand.get_rect(midleft=(text_x, center_y + 2))
        self.screen.blit(brand, brand_rect)

        # Connection status - center aligned, vertically centered
        if self.connected and 'connected' in self.icons:
            status_text = " CONNECTED"
            status_color = GREEN
            icon_status = 'connected'
        elif not self.connected and 'disconnected' in self.icons:
            status_text = " DISCONNECTED"
            status_color = RED
            icon_status = 'disconnected'
        else:
            status_text = "● CONNECTED" if self.connected else "○ DISCONNECTED"
            status_color = GREEN if self.connected else RED
            icon_status = None

        # Calculate center position for status
        status_full_width = 0
        if icon_status and icon_status in self.icons:
            status_full_width += 30  # Icon width + spacing

        status = self.font_regular.render(status_text, True, status_color)
        status_full_width += status.get_width()

        # Position status in center of screen
        status_x = (SCREEN_WIDTH - status_full_width) // 2

        if icon_status and icon_status in self.icons:
            icon_rect = self.icons[icon_status].get_rect(center=(status_x + 12, center_y + 2))
            self.screen.blit(self.icons[icon_status], icon_rect)
            status_x += 30

        status_rect = status.get_rect(midleft=(status_x, center_y + 2))
        self.screen.blit(status, status_rect)

        if self.simulate_mode:
            sim = self.font_small.render("(SIMULATION)", True, GOLD)
            sim_rect = sim.get_rect(midleft=(status_x + status.get_width() + 10, center_y + 2))
            self.screen.blit(sim, sim_rect)

        # Time - right aligned, vertically centered
        now = datetime.now()
        time_str = now.strftime("%I:%M %p")
        time_text = self.font_large.render(time_str, True, WHITE)
        time_rect = time_text.get_rect(midright=(SCREEN_WIDTH - 40, center_y + 2))
        self.screen.blit(time_text, time_rect)

    def _draw_car(self):
        if self.car_image is None:
            return

        cx = SCREEN_WIDTH // 2
        cy = SCREEN_HEIGHT // 2 - 60

        self.glow_phase += 0.03
        glow_size = 20 + 10 * math.sin(self.glow_phase)

        glow_surf = pygame.Surface((self.car_image.get_width() + glow_size * 2,
                                   self.car_image.get_height() + glow_size * 2),
                                   pygame.SRCALPHA)

        glow_center = (glow_surf.get_width() // 2, glow_surf.get_height() // 2)
        glow_radius = max(self.car_image.get_width(), self.car_image.get_height()) // 2 + glow_size

        for i in range(3):
            alpha = 30 - i * 10
            radius = glow_radius - i * 10
            pygame.draw.circle(glow_surf, (34, 211, 238, alpha), glow_center, radius)

        glow_rect = glow_surf.get_rect(center=(cx, cy))
        self.screen.blit(glow_surf, glow_rect)

        car_rect = self.car_image.get_rect(center=(cx, cy))
        self.screen.blit(self.car_image, car_rect)

    def _draw_speed(self):
        cx = SCREEN_WIDTH // 2
        cy = SCREEN_HEIGHT // 2 + 120

        speed = int(self.data.speed)

        speed_text = self.font_mega.render(str(speed), True, CYAN)
        speed_rect = speed_text.get_rect(center=(cx, cy - 20))
        self.screen.blit(speed_text, speed_rect)

        unit = self.font_medium.render("km/h", True, GRAY)
        unit_rect = unit.get_rect(center=(cx, cy + 60))
        self.screen.blit(unit, unit_rect)

        gear_text = self.font_large.render(f"GEAR {self.data.gear}", True, WHITE)
        gear_rect = gear_text.get_rect(center=(cx, cy + 105))
        self.screen.blit(gear_text, gear_rect)

    def _draw_temperature(self):
        x = 60
        y = 100
        w = 180
        h = 120

        surf = pygame.Surface((w, h), pygame.SRCALPHA)
        surf.fill((17, 24, 39, 180))
        self.screen.blit(surf, (x, y))
        pygame.draw.rect(self.screen, (255, 255, 255, 10), (x, y, w, h), 1, border_radius=12)

        # Icon
        icon_x = x + 15
        if 'temp' in self.icons:
            self.screen.blit(self.icons['temp'], (icon_x, y + 8))
            label_x = icon_x + 30
        else:
            label_x = icon_x

        label = self.font_small.render("MOTOR TEMP", True, GRAY)
        self.screen.blit(label, (label_x, y + 10))

        temp = self.data.temperature
        temp_int = int(temp)
        color = GREEN if temp < 80 else GOLD if temp < 95 else RED

        # Value with unit in same font
        temp_text = self.font_huge.render(f"{temp_int}°C", True, color)
        self.screen.blit(temp_text, (x + 15, y + 35))

        bar_x = x + 15
        bar_y = y + h - 20
        bar_w = w - 30
        bar_h = 8
        pygame.draw.rect(self.screen, (40, 40, 60), (bar_x, bar_y, bar_w, bar_h), border_radius=4)

        fill = max(0.0, min(1.0, (temp - 60.0) / 50.0))
        fill_w = int(fill * bar_w)
        if fill_w > 0:
            pygame.draw.rect(self.screen, color, (bar_x, bar_y, fill_w, bar_h), border_radius=4)

    def _draw_vibration(self):
        x = 60
        y = 240
        w = 180
        h = 120

        surf = pygame.Surface((w, h), pygame.SRCALPHA)
        surf.fill((17, 24, 39, 180))
        self.screen.blit(surf, (x, y))
        pygame.draw.rect(self.screen, (255, 255, 255, 10), (x, y, w, h), 1, border_radius=12)

        # Icon
        icon_x = x + 15
        if 'vibration' in self.icons:
            self.screen.blit(self.icons['vibration'], (icon_x, y + 8))
            label_x = icon_x + 30
        else:
            label_x = icon_x

        label = self.font_small.render("VIBRATION", True, GRAY)
        self.screen.blit(label, (label_x, y + 10))

        vib = self.data.vibration
        color = GREEN if vib < 6 else GOLD if vib < 12 else RED
        status = "NORMAL" if vib < 6 else "ATTENTION" if vib < 12 else "CRITICAL"

        vib_text = self.font_huge.render(f"{vib:.1f}", True, color)
        self.screen.blit(vib_text, (x + 15, y + 35))

        rms = self.font_regular.render("RMS", True, GRAY)
        rms_rect = rms.get_rect(midleft=(x + 15 + vib_text.get_width() + 5, y + 50))
        self.screen.blit(rms, rms_rect)

        status_text = self.font_regular.render(status, True, color)
        self.screen.blit(status_text, (x + 15, y + h - 30))

    def _draw_distance(self):
        x = SCREEN_WIDTH - 240
        y = 100
        w = 180
        h = 260

        surf = pygame.Surface((w, h), pygame.SRCALPHA)
        surf.fill((17, 24, 39, 180))
        self.screen.blit(surf, (x, y))
        pygame.draw.rect(self.screen, (255, 255, 255, 10), (x, y, w, h), 1, border_radius=12)

        # Icon
        icon_x = x + 15
        if 'distance' in self.icons:
            self.screen.blit(self.icons['distance'], (icon_x, y + 8))
            label_x = icon_x + 30
        else:
            label_x = icon_x

        label = self.font_small.render("DISTANCE", True, GRAY)
        self.screen.blit(label, (label_x, y + 10))

        dist = self.data.distance
        if dist > 500:
            display = "SAFE"
            color = GREEN
        elif dist > 200:
            display = f"{dist}mm"
            color = GOLD
        elif dist > 100:
            display = f"{dist}mm"
            color = GOLD
        else:
            display = f"{dist}mm !"
            color = RED

        dist_text = self.font_large.render(display, True, color)
        self.screen.blit(dist_text, (x + 15, y + 40))

        icon_cx = x + w // 2
        icon_cy = y + 180
        max_radius = 50
        radius = int(max_radius * (0.3 + 0.7 * (1 - min(1.0, dist / 500.0))))

        self.pulse += 0.02
        pulse_offset = int(5 * math.sin(self.pulse))

        for i, r in enumerate([20, 35, 50]):
            alpha = 30 if i == 0 else 60 if i == 1 else 90
            ring_color = (255, 255, 255, alpha)
            pygame.draw.circle(self.screen, ring_color, (icon_cx, icon_cy), r + pulse_offset * (i + 1) * 0.3, 1)

        if radius > 10:
            pygame.draw.circle(self.screen, color, (icon_cx, icon_cy), radius, 2)
            pygame.draw.line(self.screen, color, (icon_cx - radius, icon_cy), (icon_cx + radius, icon_cy), 2)
            pygame.draw.line(self.screen, color, (icon_cx, icon_cy - radius), (icon_cx, icon_cy + radius), 2)
            pygame.draw.circle(self.screen, color, (icon_cx, icon_cy), 3)

        if dist < 100:
            warn = self.font_regular.render("WARNING!", True, RED)
            warn_rect = warn.get_rect(center=(icon_cx, icon_cy + 60))
            self.screen.blit(warn, warn_rect)
        elif dist < 200:
            warn = self.font_small.render("Approaching", True, GOLD)
            warn_rect = warn.get_rect(center=(icon_cx, icon_cy + 60))
            self.screen.blit(warn, warn_rect)

    def _draw_status_bar(self):
        y = SCREEN_HEIGHT - 55
        bar_height = 40
        bar_rect = pygame.Rect(20, y, SCREEN_WIDTH - 40, bar_height)

        surf = pygame.Surface((bar_rect.width, bar_rect.height), pygame.SRCALPHA)
        surf.fill((17, 24, 39, 160))
        self.screen.blit(surf, (bar_rect.x, bar_rect.y))
        pygame.draw.rect(self.screen, (255, 255, 255, 10), bar_rect, 1, border_radius=8)

        # Center Y for vertical alignment in the bar
        center_y = bar_rect.centery - 5

        items = [
            ("SPEED", f"{int(self.data.speed)} km/h", CYAN),
            ("TEMP", f"{int(self.data.temperature)}°C", WHITE),
            ("VIBRATION", f"{self.data.vibration:.1f} RMS", WHITE),
            ("DISTANCE", f"{self.data.distance} mm", WHITE),
            ("GEAR", self.data.gear, CYAN),
        ]

        item_w = bar_rect.width // len(items)
        for i, (label, value, color) in enumerate(items):
            x = bar_rect.x + i * item_w + 20

            # Label - top of bar
            label_text = self.font_small.render(label, True, GRAY)
            label_rect = label_text.get_rect(midleft=(x, bar_rect.y + 6))
            self.screen.blit(label_text, label_rect)

            # Value - bottom of bar
            value_text = self.font_regular.render(value, True, color)
            value_rect = value_text.get_rect(midleft=(x, bar_rect.y + bar_height - 8))
            self.screen.blit(value_text, value_rect)

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_ESCAPE, pygame.K_q):
                    self.running = False
                elif event.key == pygame.K_f:
                    pygame.display.toggle_fullscreen()
                elif event.key == pygame.K_s:
                    self.simulate_mode = not self.simulate_mode
                    print(f"Simulation: {'ON' if self.simulate_mode else 'OFF'}")

    def run(self):
        while self.running:
            self.handle_events()

            self._draw_background()
            self._draw_top_bar()
            self._draw_car()
            self._draw_temperature()
            self._draw_vibration()
            self._draw_distance()
            self._draw_speed()
            self._draw_status_bar()

            pygame.display.flip()
            self.clock.tick(FPS)

        self.udp.close()
        pygame.quit()


if __name__ == "__main__":
    dashboard = EVDashboard()
    dashboard.run()