REMOTE_HOST ?= 192.168.122.195
REMOTE_USER ?= dingjing
REMOTE_PASSWORD ?=
DEB_DIR := build/deb

.PHONY: deb install-remote deploy-deb clean-deb

deb:
	sh packaging/build-deb.sh

install-remote: deb
	@test -n "$(REMOTE_PASSWORD)" || { echo "REMOTE_PASSWORD is required, e.g. REMOTE_PASSWORD=1 make install-remote"; exit 2; }
	@deb_path=$$(ls -t $(DEB_DIR)/*.deb | head -1); \
	deb_name=$$(basename "$$deb_path"); \
	echo "Uploading $$deb_name to $(REMOTE_USER)@$(REMOTE_HOST)"; \
	sshpass -p "$(REMOTE_PASSWORD)" scp -o StrictHostKeyChecking=no "$$deb_path" "$(REMOTE_USER)@$(REMOTE_HOST):/tmp/$$deb_name"; \
	echo "Installing $$deb_name on $(REMOTE_HOST)"; \
	sshpass -p "$(REMOTE_PASSWORD)" ssh -o StrictHostKeyChecking=no "$(REMOTE_USER)@$(REMOTE_HOST)" \
		"printf '%s\n' '$(REMOTE_PASSWORD)' | sudo -S apt-get install --reinstall -y /tmp/$$deb_name && \
		printf '%s\n' '$(REMOTE_PASSWORD)' | sudo -S test -f /etc/lightdm/lightdm.conf.d/50-graceful.conf && \
		printf '%s\n' '$(REMOTE_PASSWORD)' | sudo -S sed -n '1,80p' /etc/lightdm/lightdm.conf.d/50-graceful.conf && \
		ls -l /usr/bin/graceful-greeter /usr/bin/graceful-session /usr/bin/graceful-desktop /usr/bin/graceful-panel /usr/share/xgreeters/graceful-greeter.desktop /usr/share/wayland-sessions/graceful.desktop && \
		test ! -e /usr/share/xsessions/graceful.desktop"

deploy-deb: install-remote

clean-deb:
	rm -rf build/package build/deb
