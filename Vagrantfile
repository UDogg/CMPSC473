 Vagrant.configure("2") do |config|
   config.vm.provider "docker" do |d|
     d.image = "nginx:latest"
     d.ports = ["8080:80"]
     d.name = "nginx-container"
   end
 end

