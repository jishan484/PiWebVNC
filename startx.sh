#!/bin/bash
set -e


# --- Hardcoded resolution ---
RESOLUTION="1280x720"
DEPTH="24"
STATE_FILE="$HOME/.a_out_state"

if [ ! -f "$STATE_FILE" ]; then

    # --- Function to create default LXPanel config ---
    create_default_panel() {
        local PANEL_FILE="$HOME/.config/lxpanel/LXDE/panels/panel"

        # Backup existing panel config if it exists
        if [ -f "$PANEL_FILE" ]; then
            cp "$PANEL_FILE" "${PANEL_FILE}.bak"
            echo "Backup of existing panel config created at ${PANEL_FILE}.bak"
        fi

        # Create parent directories if needed
        mkdir -p "$(dirname "$PANEL_FILE")"

        # Write default panel content
        cat > "$PANEL_FILE" <<'EOF'
# Customized panel config, replace if needed

Global {
  edge=bottom
  align=left
  margin=0
  widthtype=percent
  width=100
  height=26
  transparent=0
  tintcolor=#000000
  alpha=0
  setdocktype=1
  setpartialstrut=1
  autohide=0
  heightwhenhidden=0
  usefontcolor=1
  fontcolor=#ffffff
  background=1
  backgroundfile=/usr/share/lxpanel/images/background.png
}
Plugin {
  type=space
  Config {
    Size=2
  }
}
Plugin {
  type=menu
  Config {
    image=/usr/share/lxde/images/lxde-icon.png
    system {
    }
    separator {
    }
    item {
      command=run
    }
    separator {
    }
    item {
      image=gnome-logout
      command=logout
    }
  }
}
Plugin {
  type=launchbar
  Config {
    Button {
      id=pcmanfm.desktop
    }
    Button {
      id=lxde-x-www-browser.desktop
    }
  }
}
Plugin {
  type=space
  Config {
    Size=4
  }
}
Plugin {
  type=wincmd
  Config {
    Button1=iconify
    Button2=shade
  }
}
Plugin {
  type=space
  Config {
    Size=83
  }
}
Plugin {
  type=taskbar
  expand=1
  Config {
    tooltips=1
    IconsOnly=0
    AcceptSkipPager=1
    ShowIconified=1
    ShowMapped=1
    ShowAllDesks=0
    UseMouseWheel=1
    UseUrgencyHint=1
    FlatButton=0
    MaxTaskWidth=150
    spacing=1
  }
}
Plugin {
  type=cpu
  Config {
  }
}
Plugin {
  type=tray
  Config {
  }
}
Plugin {
  type=dclock
  Config {
    ClockFmt=%R
    TooltipFmt=%A %x
    BoldFont=0
    IconOnly=0
    CenterText=0
  }
}
Plugin {
  type=launchbar
  Config {
    Button {
      id=lxde-screenlock.desktop
    }
    Button {
      id=lxde-logout.desktop
    }
  }
}
EOF

        echo "Default panel config created at $PANEL_FILE"
    }

    # Make sure your binary is executable
    chmod +x a.out

    # Update packages
    sudo apt update > /dev/null

    # Install minimal LXDE components + tools
    sudo apt install --no-install-recommends -y \
      libxfont2 x11-xkb-utils menu \
      lxsession openbox lxpanel lxappearance \
      pcmanfm lxterminal xinit x11-xserver-utils \
      mate-polkit lxde-core libfm-modules \
      menu-xdg adwaita-icon-theme lxpanel-data > /dev/null

    # --- Call function to ensure panel config exists ---
    create_default_panel
    sleep 1
    touch "$STATE_FILE"
    #==============================================================
    # --- Ensure xrandr command in ~/config/lxsession/LXDE/autostart ---
   echo -n "Dependencies Installed, Setup will start after 5s. "
    sleep 1
    echo -n "4s.. "
    sleep 1
    echo -n "3s... "
    sleep 1
    echo -n "2s.... "
    sleep 1
    echo -n "1s..... "
    sleep 1
    echo "Go!"
    AUTOSTART="$HOME/.config/lxsession/LXDE/autostart"
    mkdir -p "$(dirname "$AUTOSTART")"

    LINE="@xrandr --fb $RESOLUTION"

    # If file doesn't exist, create it with the line
    if [ ! -f "$AUTOSTART" ]; then
        cp /etc/xdg/lxsession/LXDE/autostart ~/.config/lxsession/LXDE/autostart 2>/dev/null || true
        echo "$LINE" >> "$AUTOSTART"
    else
        # If line already exists (any @xrandr), replace it
        if grep -q "^@xrandr --fb" "$AUTOSTART"; then
            sed -i "s|^@xrandr --fb.*|$LINE|" "$AUTOSTART"
        else
            # Otherwise append at the end
            echo "$LINE" >> "$AUTOSTART"
        fi
    fi

    #==============================================================

    LINE='export DISPLAY=:0'
    BASHRC="$HOME/.bashrc"
    grep -Fxq "$LINE" "$BASHRC" || echo "$LINE" >> "$BASHRC"

    # --- Force X server to use desired resolution ---
    sudo sh -c "echo 'exec ~/a.out :0 -screen 0 ${RESOLUTION}x${DEPTH}' > /etc/X11/xinit/xserverrc"

    # --- Start X + your binary ---
    startx -- ~/a.out :0 -screen 0 ${RESOLUTION}x${DEPTH} &
    PID=$!
    sleep 10
    export DISPLAY=:0

    # --- Set wallpaper ---
    pcmanfm --set-wallpaper="wallpaper.jpg" --wallpaper-mode=fit

    wait $PID
else
    # --- Start X + your binary ---
    startx -- ~/a.out :0 -screen 0 ${RESOLUTION}x${DEPTH} &
    PID=$!
    sleep 10
    export DISPLAY=:0

    # --- Set wallpaper ---
    pcmanfm --set-wallpaper="wallpaper.jpg" --wallpaper-mode=fit

    wait $PID
fi
