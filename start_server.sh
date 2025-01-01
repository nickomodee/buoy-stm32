tmux new-session -ds buoy-server sh -c "cd /home/user/buoy-stm32/; git pull; python server_code/server.py; exec bash" # -L -Logfile /home/opc/backend-server.log
