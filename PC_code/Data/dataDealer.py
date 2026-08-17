
import os
import sys
import torch
import time
import pandas as pd
import configparser
import matplotlib.pyplot as plt

# Use absolute path for config.ini
_CONFIG_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'config.ini')
config = configparser.ConfigParser()
config.read(_CONFIG_PATH)
DATA_DIR = config.get('storage', 'data_dir')

class DataStorer:
    def __init__(self,resManager,otherThreads=None):
        self.FigurePath = os.path.join(DATA_DIR, "Figure")
        self.CsvPath = os.path.join(DATA_DIR, "CSV")
        self.PTpath= os.path.join(DATA_DIR, "PT")
        self.otherThreads=otherThreads
        self.resManager=resManager

        os.makedirs(self.FigurePath, exist_ok=True)
        os.makedirs(self.CsvPath, exist_ok=True)
        os.makedirs(self.PTpath, exist_ok=True)

    def store(self,data,name,timestamp):
        csv_file_path = os.path.join(self.CsvPath, f"{name}_{timestamp}.csv")
        fig_file_path = os.path.join(self.FigurePath, f"{name}_{timestamp}.png")
        pt_file_path = os.path.join(self.PTpath, f"{name}_{timestamp}.pt")

        # Save data to CSV
        df = pd.DataFrame(data)
        df.to_csv(csv_file_path, index=False)

        # Save data to PNG
        plt.figure(figsize=(10, 5))
        plt.plot(data)
        plt.title(name)
        plt.xlabel("Time")
        plt.ylabel("Value")
        plt.savefig(fig_file_path)
        plt.close()

        # Save data to PyTorch tensor
        tensor_data = torch.tensor(data)
        torch.save(tensor_data, pt_file_path)

    def DataLoop(self):
        while True:
            #==     Getting Finished Telling ======
            if not any(thread.is_alive() for thread in self.otherThreads):
                break
            plot_data = self.resManager.plot_get()
            if plot_data is not None:
                temperature_data, vref_data = plot_data
                t=time.time()
                t_str=time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(t))
                self.store(temperature_data, "Temperature", t_str)
                self.store(vref_data, "VREF", t_str)

class DataLoader:
    def __init__(self):
        self.CsvPath = os.path.join(DATA_DIR, "CSV")
    def random_load_csv(self):
        import random
        csv_files = [f for f in os.listdir(self.CsvPath) if f.endswith('.csv')]
        csv_temp_files = [f for f in csv_files if "Temperature" in f]
        csv_vref_files = [f for f in csv_files if "VREF" in f]
        if not csv_temp_files or not csv_vref_files:
            print("No CSV files found.")
            return None, None
        temp = random.choice(csv_temp_files)
        vref = random.choice(csv_vref_files)
        return self.load_csv(temp, vref)
    def load_csv(self, temp_file, vref_file):
        temp_path = os.path.join(self.CsvPath, temp_file)
        vref_path = os.path.join(self.CsvPath, vref_file)
        temperature_data = pd.read_csv(temp_path).values.flatten()
        vref_data = pd.read_csv(vref_path).values.flatten()
        return temperature_data, vref_data

if __name__ == "__main__":
    import sys
    PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
    if PROJECT_ROOT not in sys.path:
        sys.path.insert(0, PROJECT_ROOT)
    from HOST_MODULE.KalmanFilter.Kalman_Filter import Runner
    data_loader = DataLoader()
    temperature_data, vref_data = data_loader.random_load_csv()
    if temperature_data is not None and vref_data is not None:
        runner = Runner()
        temp_est = runner.run(temperature_data[0], temperature_data)
        vref_est = runner.run(vref_data[0], vref_data)

        plt.figure(figsize=(10, 5))
        plt.plot(temperature_data, label="Temperature Data")
        plt.plot(temp_est, label="Temperature Estimate")
        plt.legend()
        plt.show()
        plt.figure(figsize=(10, 5))
        plt.plot(vref_data, label="VREF Data")
        plt.plot(vref_est, label="VREF Estimate")
        plt.legend()
        plt.show()
        plt.close()