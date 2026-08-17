
import os
import torch
import time
import pandas as pd
import configparser

config = configparser.ConfigParser()
config.read('config.ini')
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
        import matplotlib.pyplot as plt
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
        pass
